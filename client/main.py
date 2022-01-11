import sys
from app import App
import sys
import signal

def sigpipe_handler():
    print('sigpipe occured')

APPLICATION_PORT = 1234
APPLICATION_HOST = 'localhost'


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(f'Too less arguments.')
        print(f'To run application:\npython3 main.py APPLICATION_PORT APPLICATION_HOST')
        exit(0)

    # APPLICATION_PORT = sys.argv[1]
    # APPLICATION_HOST = sys.argv[2]

    app = App(APPLICATION_PORT, APPLICATION_HOST)
    signal.signal(signal.SIGPIPE, sigpipe_handler)
    sys.exit(app.app.exec_())
