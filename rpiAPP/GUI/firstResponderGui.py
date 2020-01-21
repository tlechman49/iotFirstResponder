import tkinter as tk
from tkinter import ttk
import threading
from PIL import Image

# TODO: create a node class to store sensor data and controls settings

class App(threading.Thread):

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

    def run(self):
        self.root = tk.Tk()
        self.root.title("IOT First Responder")
        self.root.resizable(width = False, height = False)
        self.root.protocol("WM_DELETE_WINDOW", self.callback)
        # self.root.pack_propagate(False)

        # paned base window
        window = ttk.PanedWindow(orient="horizontal")
        
        # sidebar
        # TODO: format sidebar
        sidebar = tk.Frame(window, width=30, bg="white", height=550)
        sidebarLabel = tk.Label(sidebar, text="Hazards")
        msdsButton = tk.Button(sidebar, width=30, text="Open MSDS in new window", command=self.dispMsdsCallback)
        floorplanButton = tk.Button(sidebar, width=30, text="Open floor plan in new window", command=self.dispFloorplanCallback)

        # main content area
        mainArea = ttk.PanedWindow(orient="vertical")

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
        controlArea = ttk.Notebook(mainArea, width=575, height=125)

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

        self.root.mainloop()


# separate thread from GUI for running tcp functions/printing to terminal
app = App()
print('***Welcome to IoT First Responder GUI***')
