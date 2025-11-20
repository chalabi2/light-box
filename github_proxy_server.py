#!/usr/bin/env python3
"""
GitHub Activity Proxy Server for ESP32 LED Panel
Fetches GitHub commit data and serves pre-processed activity arrays
"""

import os
import sys
import json
import logging
import requests
from datetime import datetime, timedelta
from flask import Flask, jsonify
from collections import defaultdict
from typing import Dict, Optional
import time

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('/var/log/github-proxy.log'),
        logging.StreamHandler(sys.stdout)
    ]
)
logger = logging.getLogger(__name__)

app = Flask(__name__)

# Configuration
GITHUB_TOKEN = os.environ.get('GITHUB_TOKEN')
if not GITHUB_TOKEN:
    logger.warning("Environment variable GITHUB_TOKEN not set; GitHub API requests will use reduced rate limits")
CACHE_DURATION = 3600  # 1 hour cache (allows daily progression while reducing API calls)
cache = {}


def build_github_headers(token: Optional[str]) -> Dict[str, str]:
    """Create headers for GitHub API requests using the provided token when available."""
    headers = {
        'Accept': 'application/vnd.github.v3+json',
        'User-Agent': 'ESP32-LED-Panel-Proxy/1.0'
    }
    if token:
        headers['Authorization'] = f'Bearer {token}'
    return headers

def fetch_github_commits(username, days=256):
    """Fetch GitHub commits for the last N days"""
    logger.info(f"Fetching GitHub commits for user: {username}")
    
    headers = build_github_headers(GITHUB_TOKEN)
    
    # Calculate date range
    end_date = datetime.now()
    start_date = end_date - timedelta(days=days)
    
    commits_by_date = defaultdict(int)
    page = 1
    max_pages = 10
    
    try:
        while page <= max_pages:
            # Use search API with date range
            url = f"https://api.github.com/search/commits"
            params = {
                'q': f'author:{username} committer-date:{start_date.strftime("%Y-%m-%d")}..{end_date.strftime("%Y-%m-%d")}',
                'sort': 'committer-date',
                'order': 'desc',
                'per_page': 100,
                'page': page
            }
            
            logger.info(f"Fetching page {page}: {url} with params: {params}")
            response = requests.get(url, headers=headers, params=params, timeout=30)
            
            if response.status_code != 200:
                logger.error(f"GitHub API error: {response.status_code} - {response.text}")
                break
                
            data = response.json()
            items = data.get('items', [])
            total_count = data.get('total_count', 0)
            
            if not items:
                logger.info(f"No more commits found on page {page}")
                break
                
            logger.info(f"Processing {len(items)} commits from page {page} (total available: {total_count})")
            
            for commit in items:
                try:
                    # Extract commit date
                    commit_date = commit['commit']['committer']['date']
                    date_obj = datetime.fromisoformat(commit_date.replace('Z', '+00:00'))
                    date_key = date_obj.strftime('%Y-%m-%d')
                    commits_by_date[date_key] += 1
                except (KeyError, ValueError) as e:
                    logger.warning(f"Error processing commit: {e}")
                    continue
            
            # Continue pagination if we have more results
            if len(items) < 100:
                logger.info(f"Page {page} returned {len(items)} items, stopping pagination")
                break
            
            # Check if we've processed enough commits for our date range
            if len(commits_by_date) >= days:
                logger.info(f"Collected {len(commits_by_date)} unique dates, sufficient for {days} day range")
                break
                
            page += 1
            time.sleep(0.2)  # Rate limiting
            
    except requests.RequestException as e:
        logger.error(f"Request error: {e}")
        return None
    
    logger.info(f"Found {len(commits_by_date)} unique dates with {sum(commits_by_date.values())} total commits")
    return commits_by_date

def process_commits_to_grid(commits_by_date, grid_size=256):
    """Convert commit data to 256-element intensity array representing last 256 days"""
    # Create a full 256-day calendar starting from 256 days ago
    end_date = datetime.now()
    start_date = end_date - timedelta(days=grid_size-1)
    logger.info(f"Generating grid for date range: {start_date.strftime('%Y-%m-%d')} to {end_date.strftime('%Y-%m-%d')}")
    intensity_array = [0] * grid_size
    
    for i in range(grid_size):
        # Calculate the date for this grid position (256 days ago to today)
        days_back = grid_size - 1 - i  # Oldest (255 days ago) to newest (0 days ago)
        target_date = end_date - timedelta(days=days_back)
        date_key = target_date.strftime('%Y-%m-%d')
        
        # Get commit count for this date
        commit_count = commits_by_date.get(date_key, 0)
        
        # Map commit count to intensity (0-4 scale)
        if commit_count == 0:
            intensity = 0
        elif commit_count == 1:
            intensity = 1
        elif commit_count <= 3:
            intensity = 2
        elif commit_count <= 6:
            intensity = 3
        else:
            intensity = 4
            
        intensity_array[i] = intensity
    
    active_days = len([x for x in intensity_array if x > 0])
    logger.info(f"Generated full 256-day calendar: {active_days} active days out of {grid_size}")
    return intensity_array

@app.route('/health')
def health_check():
    """Health check endpoint"""
    return jsonify({'status': 'healthy', 'timestamp': datetime.now().isoformat()})

@app.route('/github-activity/<username>')
def get_github_activity(username):
    """Get processed GitHub activity data for ESP32"""
    logger.info(f"Request for GitHub activity: {username}")
    
    # Check cache with date-aware key for daily progression
    current_date = datetime.now().strftime('%Y-%m-%d')
    cache_key = f"github_{username}_{current_date}"
    now = time.time()
    
    if cache_key in cache:
        cached_data, timestamp = cache[cache_key]
        if now - timestamp < CACHE_DURATION:
            logger.info(f"Returning cached data for {username} (date: {current_date})")
            return jsonify({
                'username': username,
                'data': cached_data,
                'cached': True,
                'cache_date': current_date,
                'timestamp': datetime.fromtimestamp(timestamp).isoformat()
            })
    
    # Fetch fresh data
    commits_by_date = fetch_github_commits(username)
    
    if commits_by_date is None:
        logger.error(f"Failed to fetch GitHub data for {username}")
        return jsonify({'error': 'Failed to fetch GitHub data'}), 500
    
    # Process to intensity array
    intensity_array = process_commits_to_grid(commits_by_date)
    
    # Cache the result
    cache[cache_key] = (intensity_array, now)
    
    # Return ESP32-friendly response
    response_data = {
        'username': username,
        'data': intensity_array,
        'total_commits': sum(commits_by_date.values()),
        'active_days': len([x for x in intensity_array if x > 0]),
        'cached': False,
        'timestamp': datetime.now().isoformat()
    }
    
    logger.info(f"Returning fresh data for {username}: {response_data['total_commits']} commits, {response_data['active_days']} active days")
    return jsonify(response_data)

@app.route('/github-activity/<username>/raw')
def get_github_activity_raw(username):
    """Get just the intensity array (minimal response for ESP32)"""
    logger.info(f"Raw request for GitHub activity: {username}")
    
    # Check cache with date-aware key for daily progression
    current_date = datetime.now().strftime('%Y-%m-%d')
    cache_key = f"github_{username}_{current_date}"
    now = time.time()
    
    if cache_key in cache:
        cached_data, timestamp = cache[cache_key]
        if now - timestamp < CACHE_DURATION:
            logger.info(f"Returning cached raw data for {username} (date: {current_date})")
            return jsonify(cached_data)
    
    # Fetch fresh data
    commits_by_date = fetch_github_commits(username)
    
    if commits_by_date is None:
        logger.error(f"Failed to fetch GitHub data for {username}")
        return jsonify([0] * 256)  # Return empty array on error
    
    # Process to intensity array
    intensity_array = process_commits_to_grid(commits_by_date)
    
    # Cache the result
    cache[cache_key] = (intensity_array, now)
    
    logger.info(f"Returning raw data for {username}: {len([x for x in intensity_array if x > 0])} active days")
    return jsonify(intensity_array)

if __name__ == '__main__':
    logger.info("Starting GitHub Proxy Server")
    logger.info(f"GitHub token configured: {'Yes' if GITHUB_TOKEN else 'No'}")
    
    # Run server
    app.run(
        host='0.0.0.0',  # Listen on all interfaces
        port=8080,
        debug=False,
        threaded=True
    ) 