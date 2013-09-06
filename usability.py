#!/usr/bin/python

from Tkinter import Tk,Frame,BOTH

class Test(Frame):
    def __init__(self,parent):
        Frame.__init__(self,parent,background="white")
        self.parent=parent
        self.parent.title("Usability Test")
        self.pack(fill=BOTH,expand=1)
        self.centerWindow()
    def centerWindow(self):
        w=640
        h=480
        sw=self.parent.winfo_screenwidth()
        sh=self.parent.winfo_screenheight()
        x=(sw-w)/2
        y=(sh-h)/2
        self.parent.geometry('%dx%d+%d+%d'%(w,h,x,y))

def main():
    root = Tk()
    ex=Test(root)
    root.mainloop()

if __name__ == '__main__':
    main()
