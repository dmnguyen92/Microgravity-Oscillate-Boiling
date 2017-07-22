import Tkinter as tk
import ttk as ttk 
import socket
import numpy as np

from PIL import Image, ImageTk
import PIL as PIL

# Network Variables
port = 11113
server_ip = "127.0.0.1"

# Camera Setting
noFrames = 100

# Image Setting
basefilename = "osc_bubble"
LEDduration = 2000
camsocket = 0
maxsize_image = (300, 300)

filepath = "/Users/cdohl/Library/Developer/Xcode/DerivedData/pylon_cont-awqxiahvdmhwqaapspcgpdlkmtej/Build/Products/Debug/"

def StartCamClient():
    global camsocket 
    if camsocket==0:
        camsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        camsocket.connect( (ServVar.get(), portVar.get() ) )
        camsocket.settimeout(2.0)

def CloseCamClient():
    camsocket.shutdown(socket.SHUT_RDWR)
    camsocket.close()

def SendCam(command, argument=""):
    if not argument:
        sendstring=command
    else:
        sendstring=command + " " + argument
    print("->Cam:<<" + sendstring + ">>")
    camsocket.send(sendstring)

def RecCam():
    recvd =  camsocket.recv(1024)
    if recvd[0]:
        print("received: " + recvd)
    else:
        print("timeout...")
    return recvd

def ConnectCam():
    global camsocket
    if CamConnectButton["text"]=="Connect Camera":
        CamConnectButton.config(text="Disconnect Camera")
        StartCamClient()
    else:
        CamConnectButton.config(text="Connect Camera")
        CloseCamClient()
        camsocket=0
        
def sendfeatures(event):
    print("set feature to file " + FeatureVar.get())
    SendCam("LOAD", FeatureVar.get())

def testServ():
    SendCam("FILE?")
    print("Return: " + RecCam())    

def displayImage(li):
    li.thumbnail(maxsize_image, PIL.Image.ANTIALIAS)    
    myimage = ImageTk.PhotoImage(li)
    ImagePanel.config(image=myimage)
    ImagePanel.image = myimage

def SoftTrigger():     
    StatusVar.set("Send Trigger")            
    SendCam("T") # Trigger one frame
    SendCam("FILE?") #ask for file name
    filename = RecCam()
    if (filename !="NOFILE"):
        displayImage(Image.open(filepath + filename))
        
#This reads a binary image 
#    A = np.fromfile("/Users/cdohl/Desktop/file.binary", dtype='uint8', sep="")
#    w = A[0]+A[1]*256
#    h = A[4]+A[5]*256
#    A = A[8:].reshape([h, w])
    
def on_closing():
    if camsocket:
        CloseCamClient()
    print("closing")
    win.destroy()


win = tk.Tk()
win.title("Oscillate Boiling")


#win.resizable(0, 0)

# Network Frame #######################
NetworkFrame = ttk.LabelFrame(win, text=' Camera Server ')
NetworkFrame.grid(column=0, row=0, sticky = "W")

ttk.Label(NetworkFrame, text="Port").grid(column=0, row=0)

portVar = tk.IntVar()
portVar.set(port)
portEntered = ttk.Entry(NetworkFrame, width=12, textvariable=portVar) 
portEntered.grid(column=1, row=0) 
portEntered.state(['disabled'])

ttk.Label(NetworkFrame, text="IP").grid(column=2, row=0)
ServVar = tk.StringVar()
ServVar.set(server_ip)
ServEntered = ttk.Entry(NetworkFrame, width=12, textvariable=ServVar) 
ServEntered.grid(column=3, row=0) 
ServEntered.state(['disabled'])

CamConnectButton = ttk.Button(NetworkFrame, text="Connect Camera", command=ConnectCam) 
CamConnectButton.grid(column=0, row=1)

NetButton = ttk.Button(NetworkFrame, text="Request Server", command=testServ) 
NetButton.grid(column=1, row=1)


# Camera Settings #######################
CamFrame = ttk.LabelFrame(win, text = ' Camera Settings ')
CamFrame.grid(column=0, row=1, sticky="W")


ttk.Label(CamFrame, text="Select Feature File",).grid(column=0, row=0)  # 1
FeatureVar = tk.StringVar()
FeatureEntered = ttk.Combobox(CamFrame, width=15, 
                              state='readonly',
                              values = ('FreeRun320x320.pfs',
                                        'a.psf',
                                        'FreeRun320x320.pfs'),
                              textvariable=FeatureVar)
FeatureEntered.grid(column=1, row=0)
FeatureEntered.current(1)
FeatureEntered.bind('<<ComboboxSelected>>', sendfeatures)

ttk.Label(CamFrame, text="No Frames",).grid(column=0, row=1)  # 1
noFramesVar = tk.IntVar()
noFramesVar.set(noFrames)
noFramesEntered = ttk.Entry(CamFrame, width=6, textvariable=noFramesVar) 
noFramesEntered.grid(column=1, row=1) 

# Image Setting
ISettingFrame = ttk.LabelFrame(win, text=' Image Setting ' )
ISettingFrame.grid(column=0, row=2, sticky="W")

ttk.Label(ISettingFrame, text="Base Name").grid(column=0, row=0)
ttk.Label(ISettingFrame, text="LED duration").grid(column=1, row=0)
BNameVar = tk.StringVar()
BNameVar.set(basefilename)
BNameEntered = ttk.Entry(ISettingFrame, width=12, textvariable=BNameVar) 
BNameEntered.grid(column=0, row=1) 

LEDdurationVar = tk.IntVar()
LEDdurationVar.set(LEDduration)
LEDdurationEntered = ttk.Entry(ISettingFrame, width=12, textvariable=LEDdurationVar) 
LEDdurationEntered.grid(column=1, row=1) 

# Action Buttons
action = ttk.Button(win, text="Soft Trigger!", command=SoftTrigger) 
action.grid(column=0, row=3)

StatusVar = tk.StringVar()
StatusVar.set("...")
ttk.Label(win, textvariable=StatusVar).grid(column=0, row=4)

#load image and convert
myimage = ImageTk.PhotoImage(
        Image.open("/Users/cdohl/Documents/pylon/pythongui/bubble.png"))
ImagePanel = tk.Label(win, image = myimage)
ImagePanel.grid(column=0,row=5)
#ImagePanel.pack(side = "bottom", fill = "both", expand = "yes")


win.focus_force()
win.configure(background= '#%02x%02x%02x' % (238, 238, 238))

#initialize the camera
#FeatureEntered.event_generate("<<ComboboxSelected>>")

#take care to close stuff when window gets destroyed
win.protocol("WM_DELETE_WINDOW", on_closing)

win.mainloop()
