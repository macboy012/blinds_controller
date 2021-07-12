from flask import Flask, request, abort
app = Flask(__name__)
import time
import os
import redis

SECRET = 'NO'

COMMANDS = {
        'up': 'up',
        'down': 'down',
        'upper': 'upper',
        'lower': 'lower',
        'stop': 'stop',
        'one': 'down',
        'two': 'lower',
        'too': 'lower',
        'to': 'lower',
        'three': 'upper',
        'four': 'up',
        '1': 'down',
        '2': 'lower',
        '3': 'upper',
        '4': 'up',
}

@app.route("/")
def hello():
    return "command"

@app.route("/blind_command/<command>")
def receive_commmand(command):
    secret = request.args.get('secret', None)
    if secret != SECRET:
        abort(404)

    if command in COMMANDS:
        r = redis.Redis(host='localhost', port=6379, db=0)
        r.set("blind_command", COMMANDS[command], ex=5)

    return command

MAX_WAIT = 20
@app.route("/wait_command")
def wait_command():
    r = redis.Redis(host='localhost', port=6379, db=0)
    waiter = r.get("has_waiter")
    if waiter is not None:
        now = time.time()
        print("already has waiter: %s, now: %s, diff: %s" % (float(waiter), now, now-float(waiter)))
        return "None"
    start_time = time.time()
    r.set("has_waiter", start_time, ex=MAX_WAIT)
    try:
        while time.time() < start_time + MAX_WAIT:
            val = r.get("blind_command")
            if val is not None:
                r.delete("blind_command")
                print("returning", val)
                return val
            time.sleep(0.2)
    finally:
        print("delete waiter key")
        r.delete("has_waiter")

    print("exit out of time")
    return "None"

if __name__ == "__main__":
    app.run(host='0.0.0.0')
