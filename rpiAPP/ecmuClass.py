import csv

# defining some thresholds
O2_threshold = 600
temp_threshold = 35

# defining output types to numeric values
ONBOARD_LED = 0
LED_STRIP   = 1
DOOR_SERVO  = 2


# class to store and work with individual outputs
class ecmuOutput:
    def __init__(self, outputType, pin):
        self._outputType = outputType
        self._pin = pin
        self._curMsg = 0
        self._lastMsg = 0
        
    def getOutputType(self):
        return self._outputType
        
    def getCurMsg(self):
        return self._curMsg
    
    def setCurMsg(self, msg):
        # check if a valid message is set
        if (1):
            self._lastMsg = self._curMsg
            self._curMsg = msg
            return 0
        else:
            return 1
        
    def isNewMsg(self):
        if (self._curMsg is self._lastMsg):
            return 0
        else:
            return 1
    
    def getString(self):
        return ".".join([str(self._outputType), str(self._pin), str(self._curMsg)])


# class used to store and work with individual nodes
class ecmu:
    def __init__(self, conn, addr, identifier):
        self._conn = conn
        self._addr = addr
        self._identifier = identifier
        self._O2 = 500
        self._temp = 22.0
        self._flame = 0
        self._alert = 0
        # self._currentCommand = "0.13.1000"
        # self._lastCommand = "0.13.1000"
        self._outputList = []

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
    
    def addOutput(self, outType, pin):
        self._outputList.append(ecmuOutput(outType, pin))

    # transmit output messages to node
    def transmit(self):
        separator = ""
        command = ""
        for output in self._outputList:
            if output.isNewMsg():
                command = separator.join([command, output.getString()]) # append to the command string
                output.setCurMsg(output.getCurMsg())                    # this ensures we dont resend the same command
                separator = ","                                         # set the separator (important for first run)
                
        if command != "":
            self._conn.send(command.encode())

    # receive sensor data from node
    def receive(self):
        data = self._conn.recv(1024).decode()     # data = "O2,temp,flame"
        # splitData = ["O2", "temp", "flame"]
        splitData = data.split(',')
        self._O2 = int(splitData[0])
        self._temp = float(splitData[1])
        self._flame = int(splitData[2])

    # analyze node data to perform automated operations
    def analyze(self):
        if (self._O2 > O2_threshold) or (self._temp > temp_threshold) or (self._flame == 1):
            for output in self._outputList:
                if output.getOutputType() is ONBOARD_LED:
                    output.setCurMsg(1000)
                    self._alert = 1
        else:
            for output in self._outputList:
                if output.getOutputType() is ONBOARD_LED:
                    output.setCurMsg(0)
                    self._alert = 0


# class used to store and work with the set of nodes
class ecmuSet:
    def __init__(self, maxNodes):
        self.maxNodes = maxNodes
        self.nodeList = []
        self.nodeIdDict = dict()

    # get max number of nodes
    def getMaxNodes(self):
        return self.maxNodes

    # get len of nodeList
    def getLenNodeList(self):
        return len(self.nodeList)

    # check if given identifier matches an existing node
    def isEcmu(self, identifier):
        if identifier in self.nodeIdDict.keys():
            return True
        else:
            return False

    # add a node to the list of nodes/ID LUT
    def addEcmu(self, conn, addr, identifier):
        self.nodeIdDict[identifier] = len(self.nodeList)
        self.nodeList.append(ecmu(conn, addr, identifier))

    # collect the node output assignments from a csv and append to outputList
    def collectOutputs(self):
        with open('nodeOutputAssignments.csv', newline='') as csvFile:
            reader = csv.DictReader(csvFile)
            for row in reader:
                # print(row)
                for node in self.nodeList:
                    if (int(row['identifier']) == int(node.getIdentifier())):
                        # print("found matching ID!")
                        node.addOutput(int(row['type']), int(row['pin']))                        

    # receive data from all nodes
    def receive(self):
        for node in self.nodeList:
            node.receive()

    # analyze data from all nodes
    def analyze(self):
        for node in self.nodeList:
            node.analyze()

    # transmit data from all nodes
    def transmit(self):
        for node in self.nodeList:
            node.transmit()

    # prints data from all nodes
    def print(self):
        for node in self.nodeList:
            print("Node: %s" % (node.getIdentifier()))
            print("IP Address: %s" % (node.getAddr()[0]))
            print("O2 Sensor Reading: %d" % (node.getO2()))
            print("Temperature Sensor Reading: %f" % (node.getTemp()))
            print("Fire Detected: %s" % (node.getFlame()))

            if (node.getAlert() == 1):
                print("ALERT")

            print("\n")

    def disconnect(self):
        for node in self.nodeList:
            node.getConn().close()


