#pragma once

#include "Common/json/json.hpp"
#include "Common/FSecure/WinHttp/WebProxy.h"
#include "Common/FSecure/WinHttp/Constants.h"

using json = nlohmann::json; //for easy parsing of json API: https://Gitlab.com/nlohmann/json

namespace FSecure
{
	class GitlabApi
	{
	public:

		/// Constructor for the Gitlab Api class.
		GitlabApi(std::string const& token, std::string const& channelName, std::string const& userAgent);

		/// Retrieve the Gitlab Username and initialise  for the instance
		void SetUser();

		/// set OAuth token for Gitlab
		/// @param token - the textual Gitlab OAuth token.
		void SetToken(std::string const& token);

		/// set UserAgent for Gitlab HTTP Request
		/// @param userAgent
		void SetUserAgent(std::string const& userAgent);

		/// Set the channel (i.e. Gitlab repository) that this object uses for communications
		/// @param channelName - the channel name Id (not name), for example 1.
		void SetChannel(std::string const& channelName);

		/// Will list the created repos in Gitlab and if already preset return the channel name. If not already created,
		/// creates a new repo on Gitlab.
		/// @param channelName - the actual name of the repo to create, such as "files".
		/// @return - the channel name of the new or already existing channel.
		std::string CreateChannel(std::string const& channelName);

		/// List all the repository the object's token is tied to.
		/// @return - a map of {channelName -> channelId}
		std::map<std::string, std::int64_t> ListChannels();

		/// Download file by its path.
		/// @param filename - path of file.
		/// @return - file content
		FSecure::ByteVector ReadFile(std::string const& filePath);

		/// Write a message as the contents of a file and upload to Gitlab.
		/// @param direction - the name of the file to upload
		/// @param data - the text of the message
		/// @param filename - optional custom filename for uploaded file
		void WriteMessageToFile(std::string const& direction = "", ByteView data = {}, std::string const& providedFilename = "");

		/// Upload a file in its entirety to Gitlab
		/// @param path - path to file for upload
		void UploadFile(std::string const& path);

		/// Delete a file
		/// @param filePath - the full path of the file on Gitlab.
		void DeleteFile(std::string const& filePath);

		/// Delete channel folder and all files within Gitlab
		void DeleteAllFiles();

		/// Get all of the files representing messages by a direction. This is a C3 specific method, used by a server relay to get client messages and vice versa.
		/// @param direction - the direction to search for (eg. "S2C").
		/// @return - a map of timestamp and file id, where id allows replies to be read later
		std::map<std::string, std::string> GetMessagesByDirection(std::string const& direction);


		/// Default constructor.
		GitlabApi() = default;

	private:

		/// Hold proxy settings
		WinHttp::WebProxy m_ProxyConfig;

		/// The Gitlab username
		std::string m_Username;

		/// The Gitlab channel (repo) through which messages are sent and received, will be sent when the object is created.
		std::string m_Channel;

		/// The Gitlab OAuth Token that allows the object access to the account. Needs to be manually created as described in documentation.
		std::string m_Token;

		/// UserAgent 
		std::string m_UserAgent;



		/// Send http request, uses preset token for authentication (wrapper to easily set content type)
		FSecure::ByteVector FSecure::GitlabApi::SendHttpRequest(std::string const& host, WinHttp::ContentType contentType, std::vector<uint8_t> const& data, WinHttp::Method method, bool setAuthorizationHeader = true);

		/// Send http request, uses preset token for authentication
		FSecure::ByteVector FSecure::GitlabApi::SendHttpRequest(std::string const& host, std::wstring const& contentType, std::vector<uint8_t> const& data, WinHttp::Method method, bool setAuthorizationHeader = true);

		/// Send http request, uses preset token for authentication with Gitlab accept header to view raw file content
		FSecure::ByteVector SendHttpRequest(std::string const& host, std::string const& acceptType, FSecure::WinHttp::Method method, bool setAuthorizationHeader);

		/// Send http request with json data, uses preset token for authentication
		json SendJsonRequest(std::string const& url, json const& data, WinHttp::Method method);
	};

}

