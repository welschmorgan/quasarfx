//
// Created by darkboss on 9/5/20.
//

#ifndef QUASARFX_INIPARSER_H
#define QUASARFX_INIPARSER_H

#include <core/Parser.h>
#include <core/Exception.h>
#include <core/String.h>
#include <core/Config.h>
#include "IniLexer.h"

namespace quasar {
	namespace formats {
		class IniParser
			: public core::BasicParser<Char, core::ConfigNode, IniLexer> {
		public:
			using lexer_type        = IniLexer;
			using self_type         = IniParser;
			using base_type         = core::BasicParser<Char, core::ConfigNode, lexer_type>;
			using token_type        = typename base_type::token_type;
			using id_type           = typename token_type::id_type;
			using token_list        = typename base_type::token_list;
			using result_type       = typename base_type::result_type;

			using parse_fn_type     = std::function<void(const token_type &, typename token_list::citer_type &)>;
			using parse_map_type    = std::map<id_type, parse_fn_type>;

			using section_type      = core::ConfigNode;
			using prop_store_type   = typename section_type::prop_store_type;

		protected:
			bool                                mInComment;
			bool                                mInSection;
			bool                                mInQuote;
			typename prop_store_type::iter_type mProp;
			core::BasicString<Char>             mAccu;
			result_type                         *mResult;
			parse_map_type                      mFuncs;

		public:
			IniParser()
				: mInComment(false)
				, mInSection(false)
				, mInQuote(false)
				, mProp()
				, mAccu()
				, mResult(nullptr)
				, mFuncs({
			         {lexer_type::SectionOpen.getType(), std::bind(&self_type::parseSectionOpen, this, std::placeholders::_1, std::placeholders::_2)},
			         {lexer_type::SectionClose.getType(), std::bind(&self_type::parseSectionClose, this, std::placeholders::_1, std::placeholders::_2)},
			         {lexer_type::Comment.getType(), std::bind(&self_type::parseComment, this, std::placeholders::_1, std::placeholders::_2)},
			         {lexer_type::NewLine.getType(), std::bind(&self_type::parseNewLine, this, std::placeholders::_1, std::placeholders::_2)},
			         {lexer_type::ValueAssign.getType(), std::bind(&self_type::parseAssign, this, std::placeholders::_1, std::placeholders::_2)},
			         {lexer_type::Text.getType(), std::bind(&self_type::parseText, this, std::placeholders::_1, std::placeholders::_2)},
				})
			{
			}
			IniParser(const IniParser &rhs) = default;
			virtual ~IniParser() = default;

			IniParser      &operator=(const IniParser &rhs) = default;

			void                reset() override {
				base_type::reset();
				mInComment = false;
				mInSection = false;
				mInQuote = false;
				mProp = typename prop_store_type::iter_type();
				mAccu.clear();
			}

			void                parse(const token_list &tokens, result_type &into) override {
				base_type::parse(tokens, into);
				if (!tokens.empty()) {
					auto it = tokens.end() - 1;
					if (it->getType() != lexer_type::NewLine.getType()) {
						parseNewLine(*it, it);
					}
				}
			}

			void throwEmptyPropertyName(const token_type &token);
			void throwPropertyInsertionFailed(const token_type &token);
			void throwSectionNotOnOwnLine(const token_type &token);
			void throwUnexpectedSectionOpenToken(const token_type &token);
			void throwMissingSectionStartToken(const token_type &token);
			void throwPropertyMissingKey(const token_type &token);

			void parseSectionOpen(const token_type &token, typename token_list::citer_type &it) {
				if (!mInQuote) {
					if (!mAccu.empty()) {
						throwSectionNotOnOwnLine(token);
					}
					if (mInSection) {
						throwUnexpectedSectionOpenToken(token);
					}
					mInSection = true;
				}
			}

			void parseAssign(const token_type &token, typename token_list::citer_type &it) {
				section_type *section = nullptr;
				if (!mResult->hasChildren()) {
					section = mResult;
				} else {
					section = &mResult->getChildren()->back();
				}
				mAccu.trim();
				if (mAccu.empty()) {
					throwEmptyPropertyName(token);
				}
				auto found = section->getProperties()->find(mAccu);
				if (found == section->getProperties()->end()) {
					auto inserted = section->getProperties()->insert(
							std::make_pair(mAccu, core::BasicString<Char>()));
					if (inserted.second) {
						mProp = inserted.first;
					} else {
						throwPropertyInsertionFailed(token);
					}
				} else {
					mProp = found;
				}
				mAccu.clear();
			}

			void parseSectionClose(const token_type &token, typename token_list::citer_type &it) {
				if (!mInQuote) {
					if (mInSection) {
						mAccu.trim();
						mResult->createChild(mAccu);
						mAccu.clear();
					} else {
						throwMissingSectionStartToken(token);
					}
					mInSection = false;
				}
			}

			void parseComment(const token_type &token, typename token_list::citer_type &it) {
				if (!mInQuote) {
					mInComment = true;
				}
			}

			void parseNewLine(const token_type &token, typename token_list::citer_type &it) {
				mAccu.trim();
				if (!mInComment && !mAccu.empty()) {
					if (mProp != typename prop_store_type::iter_type()) {
						mProp->second = mAccu;
					} else {
						throwPropertyMissingKey(token);
					}
				}
				mProp = typename prop_store_type::iter_type();
				mInSection = false;
				mInQuote   = false;
				mInComment = false;
				mAccu.clear();
			}

			void parseText(const token_type &token, typename token_list::citer_type &it) {
				if (!mInComment) {
					mAccu += it->getText();
				}
			}
		};
	}
}


#endif //QUASARFX_INIPARSER_H
