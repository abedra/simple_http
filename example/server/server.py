from flask import Flask
from flask import request
import json

app = Flask(__name__)

@app.route("/get")
def get():
    return json.dumps({'get': 'ok'})

@app.route("/post", methods = ['POST'])
def post():
    data = request.get_json(silent=True)
    return json.dumps({'hello': data['name']})

if __name__ == '__main__':
    app.run()