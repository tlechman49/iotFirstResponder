import tkinter as tk
from tkinter import ttk

class main_window(tk.Frame):
    def __init__(self, parent=None):
        # sidebar
        sidebar = tk.Frame(root, width=200, bg='white', height=500, relief='sunken', borderwidth=2)
        sidebar.pack(expand=False, fill='both', side='left', anchor='nw')
        sidebarLabel = tk.Label(sidebar, text="Hazards").pack()

        # main content area
        mainArea = ttk.Notebook(root, width=600, height=500)
        mainArea.pack(expand=True, fill='both', side='right')
        f1 = ttk.Frame(mainArea)
        f2 = ttk.Frame(mainArea)
        f3 = ttk.Frame(mainArea)
        f4 = ttk.Frame(mainArea)
        f5 = ttk.Frame(mainArea)
        f6 = ttk.Frame(mainArea)
        f7 = ttk.Frame(mainArea)
        mainArea.add(f1, state='disabled', text='   1 ')
        mainArea.add(f2, state='disabled', text='   2 ')
        mainArea.add(f3, state='normal', text='   3 ')
        mainArea.add(f4, state='disabled', text='   4 ')
        mainArea.add(f5, state='disabled', text='   5 ')
        mainArea.add(f6, state='disabled', text='   6 ')
        mainArea.add(f7, state='disabled', text='   7 ')

    def eventLoop(self):

        self.after(10, self.eventLoop)

if __name__ == "__main__":
    root = tk.Tk()
    root.title("IOT First Responder")
    main_window(root)
    root.mainloop()