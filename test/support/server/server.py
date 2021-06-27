from flask import Flask
from flask import request
import json

app = Flask(__name__)

@app.route('/get')
def get():
    return json.dumps({'get': 'ok'})

@app.route('/post', methods = ['POST'])
def post():
    data = request.get_json(silent=True)
    return json.dumps({'hello': data['name']})

@app.route('/empty_post_response', methods = ['POST'])
def empty_post_response():
    return '', 204

@app.route('/put', methods = ['PUT'])
def put():
    data = request.get_json(silent=True)
    return json.dumps({'update': data['update']})

@app.route('/delete', methods = ['DELETE'])
def delete():
    return '', 200

if __name__ == '__main__':
    app.run()