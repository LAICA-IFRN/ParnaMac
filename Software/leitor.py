import serial

conexao = serial.Serial("COM4", 115200, timeout=1)
comando = input("Digite L para ligar o led ou D para desligar:").upper()
while True:
    if comando == "L":
        conexao.write(b"1")
    else:
        conexao.write(b"0")
    if input("deseja continuar? S/N:").upper() == "N":
        break
conexao.close()