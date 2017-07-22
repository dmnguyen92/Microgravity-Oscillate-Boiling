// Contains an Image Event Handler that prints a message for each event method call.
#ifndef INCLUDED_IMAGEEVENTPRINTER_H_7884943
#define INCLUDED_IMAGEEVENTPRINTER_H_7884943
#include <pylon/ImageEventHandler.h>
#include <pylon/GrabResultPtr.h>
#include <iostream>
namespace Pylon
{
    class CInstantCamera;
    class CImageEventPrinter : public CImageEventHandler
    {
    public:
        virtual void OnImagesSkipped( CInstantCamera& camera, size_t countOfSkippedImages)
        {
            std::cout << "OnImagesSkipped event for device " << camera.GetDeviceInfo().GetModelName() << std::endl;
            std::cout << countOfSkippedImages  << " images have been skipped." << std::endl;
            std::cout << std::endl;
        }
        virtual void OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
        {
            std::cout << "OnImageGrabbed event for device " << camera.GetDeviceInfo().GetModelName() << std::endl;
            // Image grabbed successfully?
            if (ptrGrabResult->GrabSucceeded())
            {
                std::cout << "Grabbed frame" << std::endl;
            }
            else
            {
                std::cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << std::endl;
            }
        }
    };
}
#endif /* INCLUDED_IMAGEEVENTPRINTER_H_7884943 */
