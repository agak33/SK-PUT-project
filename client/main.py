
from app import App
import sys
import sys
import signal

def sigpipe_handler():
    print('sigpipe occured')

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(f'Not enough arguments. To run application:\n'
                'python3 main.py <application host> <application port>')
        exit(0)

    APPLICATION_PORT = None
    APPLICATION_HOST = None
    try:
        APPLICATION_HOST = sys.argv[1]
        APPLICATION_PORT = int(sys.argv[2])
    except ValueError:
        print('Wrong datatype format. App port should be an integer.')
        exit(0)

    app = App(APPLICATION_PORT, APPLICATION_HOST)
    signal.signal(signal.SIGPIPE, sigpipe_handler)
    sys.exit(app.app.exec_())
