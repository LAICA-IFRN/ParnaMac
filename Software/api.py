import serial
import json
from flask import Flask, render_template, request, jsonify
app = Flask(__name__)
contadorPessoas = 0

@app.route('/')
def hello():
    return render_template('index.html')

@app.route('/ligar')
def ligar():
    conexao = serial.Serial("COM7", 115200, timeout=1)
    conexao.write(b"1")
    conexao.close()
    return render_template('index.html',ligado="Sim")

@app.route('/desligar')
def desligar():
    conexao = serial.Serial("COM7", 115200, timeout=1)
    conexao.write(b"0")
    conexao.close()
    return render_template('index.html',ligado="Naum")

@app.route('/Ambiente', methods=['GET', 'POST'])
def ambiente():
    global contadorPessoas
    if request.method == 'POST':
        data = request.get_json()
        contadorPessoas = data.get('count', contadorPessoas)
        return jsonify({'status': 'success', 'count': contadorPessoas})
    elif request.method == 'GET':
        return render_template("ambiente.html", contadorPessoas=contadorPessoas)

if __name__ == '__main__':
   app.run(port=5000)