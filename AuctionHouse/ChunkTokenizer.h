#pragma once

#include <vector>
#include <string>

namespace auction
{
	enum class ChunkTokenizerState
	{
		inToken,
		endToken,
		endCommand,
		errTokenTooLong,
		errTooMuchTokens
	};

	class ChunkTokenizer
	{
		unsigned int maxTokens;
		unsigned int maxTokenLength;
		ChunkTokenizerState state_;
		std::vector<std::string> tokens_;
		std::string curToken;
	public:
		ChunkTokenizer(unsigned int maxTokens, unsigned int maxTokenLength) :
			maxTokens(maxTokens), maxTokenLength(maxTokenLength), state_(ChunkTokenizerState::endCommand)
		{
			curToken.reserve(maxTokenLength);
		}

		std::size_t parse(const char* data, std::size_t length);

		bool isInErrorState()
		{
			return state_ == ChunkTokenizerState::errTokenTooLong || state_ == ChunkTokenizerState::errTooMuchTokens;
		}

		void clearErrorState()
		{
			state_ = ChunkTokenizerState::endCommand;
		}

		bool isCommandReady()
		{
			return state_ == ChunkTokenizerState::endCommand;
		}

		const std::vector<std::string>& tokens()
		{
			return tokens_;
		}
	};

}
