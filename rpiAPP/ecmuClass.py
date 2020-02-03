O2_threshold = 600
temp_threshold = 35

class ecmu:

	def __init__(self, conn, addr, identifier):
		self._conn = conn
		self._addr = addr
		self._identifier = identifier
		self._O2 = 500
		self._temp = 22.0
		self._flame = 0
		self._alert = 0
		self._currentCommand = "0.13.1000"
		self._lastCommand = "0.13.1000"
		
	def getO2(self):
		return self._O2

	def getTemp(self):
		return self._temp

	def getFlame(self):
		return self._flame

	def getConn(self):
		return self._conn

	def getAddr(self):
		return self._addr

	def getIdentifier(self):
		return self._identifier

	def getAlert(self):
		return self._alert

	def transmit(self):
		if self._currentCommand != self._lastCommand:
			self._conn.send(self._currentCommand.encode())
			self._lastCommand = self._currentCommand

	def receive(self):
		data = self._conn.recv(1024).decode()     # data = "O2,temp,flame"
		splitData = data.split(',')                     # splitData = ["O2", "temp", "flame"]
		self._O2 = int(splitData[0])
		self._temp = float(splitData[1])
		self._flame = int(splitData[2])

	def analyze(self):
		if (self._O2 > O2_threshold) or (self._temp > temp_threshold) or (self._flame == 1):
			self._currentCommand = "0.13.1000"       # 1 -> LED on
			self._alert = 1
		else:
			self._currentCommand = "0.13.0"       # 0 -> LED off
			self._alert = 0