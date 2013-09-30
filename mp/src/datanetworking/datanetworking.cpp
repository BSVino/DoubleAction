#include <string>
#include <sstream>
#include <time.h>
#include <vector>

extern "C" {
#include <curl/curl.h>
}

#undef min
#undef max

#include "../datanetworking/math.pb.h"
#include "../datanetworking/data.pb.h"

using std::string;
using std::stringstream;

size_t CurlReadFunction(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	stringstream& ssData = *(stringstream*)userdata;
	string sData = ssData.str();

	size_t iBytesToRead = sData.size() - ssData.tellp();
	if (iBytesToRead > nmemb)
		iBytesToRead = nmemb;

	memcpy(ptr, sData.data() + ssData.tellp(), iBytesToRead);
	ssData.seekp((size_t)ssData.tellp() + iBytesToRead);

	return iBytesToRead;
}

void DebugString(const char* pszString)
{
#ifdef __linux__
    printf("%s", pszString);
#else
    OutputDebugString(pszString); // Sends it to the VS debug window
#endif
}

int CurlDebugCallback(CURL *pCurl, curl_infotype eInfo, char* psz, size_t i, void* pData)
{
	if (eInfo == CURLINFO_TEXT)
		DebugString("Info: ");
	else if (eInfo == CURLINFO_HEADER_IN)
		DebugString("Header in: ");
	else if (eInfo == CURLINFO_HEADER_OUT)
		DebugString("Header out: ");
	else if (eInfo == CURLINFO_DATA_IN)
		DebugString("Data in: ");
	else if (eInfo == CURLINFO_DATA_OUT)
		DebugString("Data out: ");

	DebugString(psz);

	return 0;
}

bool DASendData(const da::protobuf::GameData& pbGameData, string& sError)
{
	bool bSuccess = false;

	curl_global_init(CURL_GLOBAL_ALL);

	string sData;

	if (pbGameData.SerializeToString(&sData))
	{
		stringstream ssData(sData);

		CURL* pCurl = curl_easy_init();

		if (pCurl)
		{
			curl_easy_setopt(pCurl, CURLOPT_URL, "http://data.doubleactiongame.com/data/receive.php");
			curl_easy_setopt(pCurl, CURLOPT_UPLOAD, 1);
			curl_easy_setopt(pCurl, CURLOPT_READDATA, &ssData);
			curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, CurlReadFunction);
			curl_easy_setopt(pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)sData.size());

#ifdef _DEBUG
			//curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
			//curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, CurlDebugCallback);
#endif

			CURLcode eRes = curl_easy_perform(pCurl);

			if(eRes == CURLE_OK)
				bSuccess = true;
			else
				sError = string("curl_easy_perform() failed: ") + curl_easy_strerror(eRes) + "\n";

			curl_easy_cleanup(pCurl);
		}
		else
			sError = "curl_easy_init() failed.\n";
	}
	else
		sError = "Data serialization failed.\n";

	curl_global_cleanup();

	return bSuccess;
}
