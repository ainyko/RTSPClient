#include "StdAfx.h"
#include "ourRTSPClient.h"

CourRTSPClient::~CourRTSPClient(void)
{
}


// Implementation of "ourRTSPClient":

CourRTSPClient* CourRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
	int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) {
		return new CourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);


}

CourRTSPClient::CourRTSPClient(UsageEnvironment& env, char const* rtspURL,   int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) : RTSPClient(env,rtspURL, verbosityLevel, applicationName,tunnelOverHTTPPortNum, -1)
{
}