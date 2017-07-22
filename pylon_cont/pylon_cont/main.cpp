// Grab_UsingGrabLoopThread.cpp
/*
 Note: Before getting started, Basler recommends reading the Programmer's Guide topic
 in the pylon C++ API documentation that gets installed with pylon.
 If you are upgrading to a higher major version of pylon, Basler also
 strongly recommends reading the Migration topic in the pylon C++ API documentation.
 This sample illustrates how to grab and process images using the grab loop thread
 provided by the Instant Camera class.
 */
// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>

// Include files used by samples.
#include "include/ConfigurationEventPrinter.h"
#include "include/ImageEventPrinter.h"

// Include files used by server
#include <stdio.h>
#include <string.h>     //strlen, strtok
#include <string>       //conversion to Cpp strings and some operations thereof
#include <sys/socket.h>
#include <arpa/inet.h>  //inet_addr
#include <unistd.h>     //write


#define MSG_LENGTH 50

// Namespace for using pylon objects.
using namespace Pylon;
// Namespace for using cout.
using namespace std;
// continuous frame counter;
uint32_t frameno = 0;
int client_sock;
string current_filename="NOFILE";


//image event handler.
class CSampleImageEventHandler : public CImageEventHandler
{
public:
    virtual void OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
    {
        
        FILE* pFile;
        uint32_t w,h;
        cout << "CSampleImageEventHandler::OnImageGrabbed called." << std::endl;
        char frameFilename[150];
        uint16_t frameNumber = (uint16_t)ptrGrabResult->GetBlockID();
        sprintf(frameFilename,"GrabbedImage_%.2d.png",frameNumber);
        CImagePersistence::Save( ImageFileFormat_Png, frameFilename, ptrGrabResult);
        
        cout << "Saved image to file: " << frameFilename << endl << endl;
        current_filename = frameFilename;
        //Binary save seems faster
        w=ptrGrabResult->GetWidth();
        h=ptrGrabResult->GetHeight();
        pFile = fopen("file.binary", "wb");
        fwrite(&w, 1, sizeof(w), pFile);
        fwrite(&h, 1, sizeof(h), pFile);
        fwrite(ptrGrabResult->GetBuffer(), 1, ptrGrabResult->GetHeight()*ptrGrabResult->GetWidth(), pFile);
        fclose(pFile);
    }
};


// Check if cpp string consist only of spaces
bool has_only_spaces(std::string& str) {
    return str.find_first_not_of (' ') == str.npos;
}

// Send data
bool tcp_send_data(string data){
    if( send(client_sock , data.c_str() , strlen( data.c_str() ) , 0) < 0){
        perror("Send failed : ");
        return false;
    }
    cout<<"Data send\n";
    return true;
}

int main(int argc, char* argv[])
{
    int socket_desc, c , read_size;
    struct sockaddr_in server, client;
    char client_message[MSG_LENGTH];
    char * cmd;
    char * arg;
    string Cpparg, Cppcmd;
    int err;
    bool end_grab = false;

    // The exit code of the sample application.
    int exitCode = 0;
    // Before using any pylon methods, the pylon runtime must be initialized.
    PylonInitialize();

    try
    {
        // Create an instant camera object for the camera device found first.
        CInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice());
        // Register the standard configuration event handler for enabling software triggering.
        // The software trigger configuration handler replaces the default configuration
        // as all currently registered configuration handlers are removed by setting the registration mode to RegistrationMode_ReplaceAll.
        camera.RegisterConfiguration( new CSoftwareTriggerConfiguration, RegistrationMode_ReplaceAll, Cleanup_Delete);
        // For demonstration purposes only, add a sample configuration event handler to print out information
        // about camera use.
        camera.RegisterConfiguration( new CConfigurationEventPrinter, RegistrationMode_Append, Cleanup_Delete);
        // The image event printer serves as sample image processing.
        // When using the grab loop thread provided by the Instant Camera object, an image event handler processing the grab
        // results must be created and registered.
        //camera.RegisterImageEventHandler( new CImageEventPrinter, RegistrationMode_Append, Cleanup_Delete);
        // For demonstration purposes only, register another image event handler.
        camera.RegisterImageEventHandler( new CSampleImageEventHandler, RegistrationMode_Append, Cleanup_Delete);
        // Open the camera device.
        camera.Open();
        
        //Create socket
        socket_desc = socket(AF_INET , SOCK_STREAM , 0);
        if (socket_desc == -1)
        {
            printf("Could not create socket");
        }
        puts("Socket created");
        //Prepare the sockaddr_in structure
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons( 11113 );
        
        //Bind
        if( ::bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
        {
            //print the error message
            perror("bind failed. Error");
            return 1;
        }
        puts("bind done");
        
        //Listen
        listen(socket_desc , 3);
        
        //Accept and incoming connection
        puts("Waiting for incoming connections...");
        c = sizeof(struct sockaddr_in);
        
        //accept connection from an incoming client
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
        if (client_sock < 0)
        {
            perror("accept failed");
            throw ("accept failed exception.");
        }
        puts("Connection accepted");
        
        
        // Can the camera device be queried whether it is ready to accept the next frame trigger?
        if (camera.CanWaitForFrameTriggerReady())
        {
            // Start the grabbing using the grab loop thread, by setting the grabLoopType parameter
            // to GrabLoop_ProvidedByInstantCamera. The grab results are delivered to the image event handlers.
            // The GrabStrategy_OneByOne default grab strategy is used.
            camera.StartGrabbing( GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
            
            cout << endl << "Enter \"t\" to trigger the camera or \"e\" to exit and press enter? (t/e)" << endl << endl;
            
            // Wait for user input to trigger the camera or exit the program.
            // The grabbing is stopped, the device is closed and destroyed automatically when the camera object goes out of scope.
            do{
                
                memset(client_message, 0, MSG_LENGTH);
                read_size = (int) recv(client_sock, client_message , 50 , 0);
                err = errno;
                if (read_size<=0){
                    throw ("recv returned unrecoverable error.");
                }
                //Message parser into command and argument
                printf("Msg:<<%s>>\n",client_message);
                arg = NULL;
                cmd = strtok (client_message, " ");
                arg = strtok (NULL, "");
                
                
                printf("cmd: %s\n",cmd);
                printf("arg: %s\n",arg);
                Cppcmd = cmd;
                if (arg != NULL){
                    Cpparg = arg;
                }
                
                if (has_only_spaces(Cpparg))
                    Cpparg = "";// if only spaces argument is ignored
                
                if (Cppcmd == "LOAD"){
                    printf ("Load command obtained.\n");
                    if (!Cpparg.empty()){
                        cout << "with argument: " << (string) arg << endl;
                        camera.StopGrabbing();
                        cout << "Camera Stopped Grabbing" << endl;
                        CFeaturePersistence::Load( arg, &camera.GetNodeMap(), true );
                        cout << "Loaded new Feature File" << endl;
                        camera.StartGrabbing( GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
                        cout << "Camera Started Grabbing" << endl;
                    }
                } else if (Cppcmd == "T"){
                    // Execute the software trigger. Wait up to 500 ms for the camera to be ready for trigger.
                    if ( camera.WaitForFrameTriggerReady( 500, TimeoutHandling_ThrowException))
                    {
                        camera.ExecuteSoftwareTrigger();
                    }
                } else if (Cppcmd == "FILE?"){
                    tcp_send_data(current_filename);
                } else if (Cppcmd == "STOP"){
                    end_grab = true;
                } else {
                    printf ("command not recognized...\n");
                }
            } while (!end_grab);
        }
        else
        {
            // See the documentation of CInstantCamera::CanWaitForFrameTriggerReady() for more information.
            cout << endl;
            cout << "This sample can only be used with cameras that can be queried whether they are ready to accept the next frame trigger.";
            cout << endl;
            cout << endl;
        }
    }
    catch (const GenericException &e)
    {
        // Error handling.
        cerr << "An exception occurred." << endl << e.GetDescription() << endl;
        exitCode = 1;
        // Remove left over characters from input buffer.
        cin.ignore(cin.rdbuf()->in_avail());
    }
    catch (const char *msg)
    {
        cout << msg << endl;
        shutdown (socket_desc, 2);
        close(socket_desc);
    }
    close(client_sock);
    
    // Releases all pylon resources.
    PylonTerminate();
    
    return exitCode;
    
}
