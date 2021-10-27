from flask import Flask, render_template, request
import atexit
import RPi.GPIO as GPIO

app = Flask(__name__)

@app.route('/')
def root():
    print('/root')
    return 'ROOT'

@app.route('/page')
@app.route('/page/<name>')
def page(name=None):
    print('/page')
    return render_template('test.html', name=name)


@app.route('/post', methods=['GET', 'POST'])
def post():
    if request.method == 'POST':
        logfile = open("logging/test.txt", "a")
        logfile.write("Testi\n")
        logfile.close()
        return 'Wrote to log file from POST request'
    else:
        return 'GET request'


def app_close():
    GPIO.cleanup()

atexit.register(app_close)
