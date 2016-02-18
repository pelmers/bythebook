#!/usr/bin/env python

import os
import os.path
import glob
from random import randrange
import shutil
import subprocess
import urllib
import json

from flask import Flask, request, send_file, jsonify

# Prepare environment.
app = Flask(__name__)
env = json.loads(open(os.path.join(os.path.dirname(__file__), "env.json")).read())

# Define constants.
SPINE_CROP_BIN = env["SPINE_CROP_BIN"]
TESSERACT_BIN = env["TESSERACT_BIN"]
GOOGLE_API_KEY = env["GOOGLE_API_KEY"]
GOOGLE_CUSTOM_SEARCH_ID = "017566473173564839188%3Aenujydcz7ri"
assert(os.path.exists(SPINE_CROP_BIN))


def filter_english(text):
    # TODO so we don't waste precious google queries on nonsense
    patterns = {
    }
    for pattern in patterns:
        text = text.replace(pattern, "")
    return text


@app.route("/")
def index():
    return app.send_static_file("index.html")


@app.route("/sessions/<session_id>/<session_file>")
def send_session_file(session_id, session_file):
    return send_file("sessions/%s/%s" % (session_id, session_file))


@app.route("/post/", methods={"POST"})
def post():
    session_id = randrange(100000)
    # Save the image.
    image_file = request.files['image']
    session_dir = "sessions/%d" % session_id
    if os.path.exists(session_dir):
        shutil.rmtree(session_dir)
    os.mkdir(session_dir)
    save_loc = "%s/%s" % (session_dir, image_file.filename)
    request.files['image'].save(save_loc)
    # Run bookview on it.
    spine_crop_popen = subprocess.Popen([SPINE_CROP_BIN, image_file.filename], cwd=session_dir)
    spine_crop_popen.communicate()
    # Feed outputs to tesseract.
    crops = glob.glob(session_dir + "/cropped*.png")
    titles = []
    for crop in crops:
        cmd = subprocess.Popen([TESSERACT_BIN, crop, "stdout"], stdout=subprocess.PIPE)
        output = cmd.communicate()[0].decode('utf-8').encode('ascii', errors='ignore')
        output = filter_english(u' '.join(output.splitlines()))
        if output:
            titles.append({'img': crop, 'results': search(output), 'text': output})
    # Return output JSON.
    return jsonify({'titles': titles})


def search(query_term, number_results=3):
    query = urllib.urlencode({'q': query_term})
    url = 'https://www.googleapis.com/customsearch/v1?&cx=%s&key=%s&%s' % (GOOGLE_CUSTOM_SEARCH_ID, GOOGLE_API_KEY, query)
    print "Querying", url
    search_response = urllib.urlopen(url)
    search_results = search_response.read()
    results = json.loads(search_results)
    if int(results['searchInformation']['totalResults']) > 0:
        return [{'link': res['link'], 'title': res['title']} for res in
                results['items'][:number_results]]
    else:
        return []


if __name__ == '__main__':
    app.run( debug=True)
