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

static void DebugString(const char* pszString)
{
#ifdef _WIN32
    OutputDebugString(pszString); // Sends it to the VS debug window
#else
    printf("%s", pszString);
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
			curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 10L);
			curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);
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

static int still_running = 0;
static CURL* curl;
static CURLM* curlm;

static string date;
static bool have_data = false;

size_t CurlWriteDate(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	date.append(ptr, size*nmemb);
	have_data = true;
	return size*nmemb;
}

void DAFetchMostRecentNews()
{
	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();
	curlm = curl_multi_init();

	if (!curl || !curlm)
	{
		curl_global_cleanup();
		return;
	}

	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(curl, CURLOPT_URL, "http://forums.doubleactiongame.com/latest.php");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteDate);

#ifdef _DEBUG
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	//curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, CurlDebugCallback);
#endif

	curl_multi_add_handle(curlm, curl);

	curl_multi_perform(curlm, &still_running);
}

static void explode(const string& str, std::vector<string>& tokens, const string& delimiter = " ")
{
	string::size_type lastPos = str.find_first_of(delimiter, 0);
	string::size_type pos = 0;

	while (true)
	{
		tokens.push_back(str.substr(pos, lastPos - pos));

		if (lastPos == string::npos)
			break;

		pos = lastPos+1;
		lastPos = str.find_first_of(delimiter, pos);
	}
}

bool DAMostRecentNewsReady(int& most_recent_news, int& most_recent_version)
{
	if (have_data)
	{
		std::vector<string> values;
		explode(date, values);

		if (values.size() >= 2)
		{
			most_recent_news = atoi(values[0].c_str());
			most_recent_version = atoi(values[1].c_str());
			return true;
		}
		else
			return false;
	}

	if (still_running == 0)
		return false;

	curl_multi_perform(curlm, &still_running);

	if (still_running == 0)
	{
		curl_multi_cleanup(curlm);
		curl_easy_cleanup(curl);

		curl_global_cleanup();
	}

	return false;
}
