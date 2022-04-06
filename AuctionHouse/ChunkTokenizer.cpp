#include "ChunkTokenizer.h"

namespace auction
{

	std::size_t ChunkTokenizer::parse(const char* data, std::size_t length)
	{
		for (const char* p = data; p < data + length; ++p)
		{
			switch (*p)
			{
			case '\r':
				break;
			case ' ':
			case '\t':
				if (state_ == ChunkTokenizerState::endCommand || state_ == ChunkTokenizerState::endToken
					|| isInErrorState())
				{
					continue;
				}
				if (tokens_.size() == maxTokens)
				{
					state_ = ChunkTokenizerState::errTooMuchTokens;
					continue;
				}
				state_ = ChunkTokenizerState::endToken;
				tokens_.push_back(curToken);
				curToken.clear();
				break;
			case '\n':
				if (state_ == ChunkTokenizerState::inToken)
				{
					if (tokens_.size() == maxTokens)
					{
						state_ = ChunkTokenizerState::errTooMuchTokens;
					}
					else
					{
						tokens_.push_back(curToken);
					}
				}
				if (!isInErrorState())
				{
					state_ = ChunkTokenizerState::endCommand;
				}
				curToken.clear();
				return p - data + 1;
			default:
				if (isInErrorState())
				{
					continue;
				}
				if (state_ == ChunkTokenizerState::endCommand)
				{
					tokens_.clear();
				}
				if (curToken.length() == maxTokenLength)
				{
					state_ = ChunkTokenizerState::errTokenTooLong;
					continue;
				}
				state_ = ChunkTokenizerState::inToken;
				curToken.push_back(*p);
				break;
			}
		}
		return length;
	}

}
