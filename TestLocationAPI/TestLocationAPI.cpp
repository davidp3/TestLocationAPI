// TestLocationAPI.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <atlbase.h>
#include <atlcom.h>
#include <LocationApi.h> // This is the main Location API header
#include <shlwapi.h>

#pragma warning(disable : 4995)
struct LocationEvent final : public ILocationEvents {
public:

  // IUnknown interface
  STDMETHODIMP_(ULONG) AddRef() override;
  STDMETHODIMP_(ULONG) Release() override;
  STDMETHODIMP QueryInterface(REFIID iid, void** ppv) override;

  // ILocationEvents interface
  STDMETHODIMP OnStatusChanged(REFIID aReportType,
      LOCATION_REPORT_STATUS aStatus) override;
  STDMETHODIMP OnLocationChanged(REFIID aReportType,
    ILocationReport* aReport) override;

private:
  ULONG mCount = 0;
};

STDMETHODIMP_(ULONG)
LocationEvent::AddRef() { return InterlockedIncrement(&mCount); }

STDMETHODIMP_(ULONG)
LocationEvent::Release() {
  ULONG count = InterlockedDecrement(&mCount);
  if (!count) {
    delete this;
    return 0;
  }
  return count;
}

STDMETHODIMP
LocationEvent::QueryInterface(REFIID iid, void** ppv) {
  if (iid == IID_IUnknown) {
    *ppv = static_cast<IUnknown*>(this);
  }
  else if (iid == IID_ILocationEvents) {
    *ppv = static_cast<ILocationEvents*>(this);
  }
  else {
    return E_NOINTERFACE;
  }
  AddRef();
  return S_OK;
}

STDMETHODIMP
LocationEvent::OnStatusChanged(REFIID aReportType,
  LOCATION_REPORT_STATUS aStatus) {
  printf("LocationEvent::OnStatusChanged\n");
  return S_OK;
}

STDMETHODIMP
LocationEvent::OnLocationChanged(REFIID aReportType, ILocationReport* aReport) {
  printf("LocationEvent::OnLocationChanged\n");
  return S_OK;
}

class CInitializeATL : public CAtlExeModuleT<CInitializeATL> {};
CInitializeATL g_InitializeATL; // Initializes ATL for this application. This also does CoInitialize for us

int wmain()
{
  // Run in MTA:
  // HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
  // Run in STA:
  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  if (SUCCEEDED(hr))
  {
    CComPtr<ILocation> spLocation; // This is the main Location interface
    LocationEvent* pLocationEvents = NULL; // This is our callback object for location reports
    IID REPORT_TYPES[] = { IID_ILatLongReport }; // Array of report types of interest. Other ones include IID_ICivicAddressReport

    hr = spLocation.CoCreateInstance(CLSID_Location); // Create the Location object

    if (SUCCEEDED(hr))
    {
      pLocationEvents = new LocationEvent();
      if (NULL != pLocationEvents)
      {
        pLocationEvents->AddRef();
      }
    }

    if (SUCCEEDED(hr))
    {
      // Request permissions for this user account to receive location data for all the
      // types defined in REPORT_TYPES (which is currently just one report)
      if (FAILED(spLocation->RequestPermissions(NULL, REPORT_TYPES, ARRAYSIZE(REPORT_TYPES), FALSE))) // FALSE means an asynchronous request
      {
        wprintf(L"Warning: Unable to request permissions.\n");
      }

      // Tell the Location API that we want to register for reports (which is currently just one report)
      for (DWORD index = 0; index < ARRAYSIZE(REPORT_TYPES); index++)
      {
        hr = spLocation->RegisterForReport(pLocationEvents, REPORT_TYPES[index], 0);
      }
    }

    if (SUCCEEDED(hr))
    {
      // Wait until user presses a key to exit app. During this time the Location API
      // will send reports to our callback interface on another thread (if running in MTA).
      system("pause");

      // Unregister from reports from the Location API
      for (DWORD index = 0; index < ARRAYSIZE(REPORT_TYPES); index++)
      {
        spLocation->UnregisterForReport(REPORT_TYPES[index]);
        // DLP TEST: Double UnregisterForReport
        spLocation->UnregisterForReport(REPORT_TYPES[index]);
      }
    }

    // Cleanup
    if (NULL != pLocationEvents)
    {
      pLocationEvents->Release();
      pLocationEvents = NULL;
    }

  }

  CoUninitialize();
  return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
