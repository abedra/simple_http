from flask import Flask
from flask import request
from flask import Response
import json

app = Flask(__name__)

@app.route('/get')
def get():
    return json.dumps({'get': 'ok'})

@app.route('/get_hello')
def get_hello():
    return json.dumps({'hello': request.args.get('name')})

@app.route('/get_full')
def get_full():
    return json.dumps({
        'first': request.args.get('first'),
        'last': request.args.get('last')
    })

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

@app.route('/trace', methods = ['TRACE'])
def trace():
    return Response(request.data, status=200, mimetype='message/http')

if __name__ == '__main__':
    app.run()