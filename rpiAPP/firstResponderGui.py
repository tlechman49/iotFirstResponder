#!/usr/bin/env python

import tkinter as tk
from tkinter import ttk
import threading
from PIL import Image
import socket
from time import sleep

import ecmuClass

TCP_IP = '0.0.0.0'
TCP_PORT = 5005
BUFFER_SIZE = 1024

nodes = []
nodeDict = dict()

O2_threshold = 600
temp_threshold = 35

initiated = 0

class App(threading.Thread):

    def initiateNodes(self):
        global initiated, nodes, nodeDict
        number_of_nodes = int(self.nodesVar.get())
        print("Initiating " + str(number_of_nodes) + " node(s)...")
        self.ecmuSet = ecmuClass.ecmuSet(number_of_nodes)

        # Initialize TCP Server socket
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind((TCP_IP, TCP_PORT))

        # Three-way Handshake with each TCP Client, build node array
        while self.ecmuSet.getLenNodeList() < self.ecmuSet.getMaxNodes():
            s.listen(1)
            conn, addr = s.accept()
            identifier = conn.recv(BUFFER_SIZE).decode()    # use a portion of the node's MAC address as an identifier
            if self.ecmuSet.isEcmu(identifier):
                pass
            else:
                self.ecmuSet.addEcmu(conn, addr, identifier)

        initiated = 1
        print("Initiated!")

    def __init__(self):
        threading.Thread.__init__(self)
        self.start()

    def callback(self):
        self.root.quit()

    def dispMsdsCallback(self):
        img = Image.open('Third_Floor_Plan_Large.png')
        img.show()

    def dispFloorplanCallback(self):
        img = Image.open('Third_Floor_Plan_Large.png')
        img.show()

    def createCurrentWindow(self):
        self.currentWindow = tk.Toplevel(self.root)
        self.currentWindow.title("IOT First Responder")
        self.currentWindow.resizable(width = False, height = False)
        self.currentWindow.protocol("WM_DELETE_WINDOW", self.callback)

    def createInitWindow(self):
        self.createCurrentWindow()
        localIP = socket.gethostbyname(socket.gethostname())
        label = tk.Label(self.currentWindow, text="Choose settings for server at " + localIP)
        nodesLabel = tk.Label(self.currentWindow, text="Enter number of nodes:")
        self.nodesVar = tk.StringVar(self.currentWindow, value='1')
        nodesEntry = tk.Entry(self.currentWindow, textvariable=self.nodesVar, width=4)
        button = tk.Button(self.currentWindow, text="Start", command=self.createMainWindow)

        label.grid(row=0, column=0, columnspan=2)
        nodesLabel.grid(row=1, column=0, sticky="w")
        nodesEntry.grid(row=1, column=1)
        button.grid(row=2, column=0, columnspan=2)

    def createMainWindow(self):

        # comment out to bypass start (also uncomment the line in run)
        self.initiateNodes()
        self.currentWindow.destroy()

        self.createCurrentWindow()

        # paned base window
        window = ttk.PanedWindow(self.currentWindow, orient="horizontal")

        # sidebar
        # TODO: format sidebar
        sidebar = tk.Frame(window, width=30, bg="white", height=550)
        sidebarLabel = tk.Label(sidebar, text="Hazards")
        msdsButton = tk.Button(sidebar, width=30, text="Open MSDS in new window", command=self.dispMsdsCallback)
        floorplanButton = tk.Button(sidebar, width=30, text="Open floor plan in new window", command=self.dispFloorplanCallback)

        # main content area
        mainArea = ttk.PanedWindow(window, orient="vertical")

        # floorplan area
        # TODO: insert node buttons
        fpArea = ttk.Notebook(mainArea, width=575, height=425)
        floorPlan = tk.PhotoImage(file='floorplan.png')
        f1 = ttk.Frame(mainArea)
        f2 = ttk.Frame(mainArea)
        f3 = tk.Canvas(mainArea)
        f3.background = floorPlan
        bg = f3.create_image(0,0, anchor = tk.NW, image = floorPlan)
        f4 = ttk.Frame(mainArea)
        f5 = ttk.Frame(mainArea)
        f6 = ttk.Frame(mainArea)
        f7 = ttk.Frame(mainArea)

        # controls and sensor readings for nodes
        # TODO: bring out sensor readings add controls as they are decided
        controlArea = ttk.Frame(mainArea, width=575, height=125)
        self.idVar = tk.StringVar(self.currentWindow, value='Node 69')
        idVarLabel = tk.Label(self.currentWindow, textvariable=self.idVar)
        tempLabel = tk.Label(self.currentWindow, text="Temperature (C): ")
        self.tempVar = tk.StringVar(self.currentWindow, value='22')
        tempVarLabel = tk.Label(self.currentWindow, textvariable=self.tempVar)
        aqLabel = tk.Label(self.currentWindow, text="Air Quality (PPM): ")
        self.aqVar = tk.StringVar(self.currentWindow, value='400')
        aqVarLabel = tk.Label(self.currentWindow, textvariable=self.aqVar)
        nirLabel = tk.Label(self.currentWindow, text="NIR Fire Detected: ")
        self.nirVar = tk.StringVar(self.currentWindow, value='False')
        nirVarLabel = tk.Label(self.currentWindow, textvariable=self.nirVar)

        # packing / adding the components onto the display
        window.pack(fill="both", expand = False)

        # sidebar
        window.add(sidebar)
        sidebarLabel.pack()
        msdsButton.pack()
        floorplanButton.pack()

        # main area
        window.add(mainArea)

        mainArea.add(fpArea)
        fpArea.add(f1, state='disabled', text='   1 ')
        fpArea.add(f2, state='disabled', text='   2 ')
        fpArea.add(f3, state='normal', text='   3 ')
        fpArea.add(f4, state='disabled', text='   4 ')
        fpArea.add(f5, state='disabled', text='   5 ')
        fpArea.add(f6, state='disabled', text='   6 ')
        fpArea.add(f7, state='disabled', text='   7 ')

        mainArea.add(controlArea)
        idVarLabel.grid(in_=controlArea, row=1, column=0, stick="w")
        tempLabel.grid(in_=controlArea, row=2, column=0, sticky="w")
        tempVarLabel.grid(in_=controlArea, row=2, column=1)
        aqLabel.grid(in_=controlArea, row=3, column=0, sticky="w")
        aqVarLabel.grid(in_=controlArea, row=3, column=1)
        nirLabel.grid(in_=controlArea, row=4, column=0, sticky="w")
        nirVarLabel.grid(in_=controlArea, row=4, column=1)

    def run(self):
        self.root = tk.Tk()
        self.root.withdraw()

        # comment out init window and uncomment main for debugging the main window
        self.createInitWindow()
        # self.createMainWindow()

        self.root.mainloop()


if __name__ == "__main__":
    # begins GUI thread
    app = App()

    print('***Welcome to IoT First Responder GUI***')
    while not initiated:
        sleep(1)

    # Loop reading sensor data, displaying sensor data, and sending commands to node.  Break loop with Ctrl-C
    try:
        while True:
            app.ecmuSet.receive()
            app.ecmuSet.analyze()
            # app.ecmuSet.print()
            for node in app.ecmuSet.nodeList:
                app.idVar.set("Node " + str(node.getIdentifier()))
                app.tempVar.set(str(node.getTemp()))
                app.aqVar.set(str(node.getO2()))
                app.nirVar.set("False" if node.getFlame() == 0 else "True")

            app.ecmuSet.transmit()
            # sleep(1)

    except KeyboardInterrupt:
        app.ecmuSet.disconnect()