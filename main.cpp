#include <algorithm>
#include <iostream>
#include <set>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine()
{
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber()
{
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text)
{
    vector<string> words;
    string word;
    for (const char c : text)
    {
        if (c == ' ')
        {
            if (!word.empty())
            {
                words.push_back(word);
                word.clear();
            }
        }
        else
        {
            word += c;
        }
    }
    if (!word.empty())
    {
        words.push_back(word);
    }

    return words;
}

struct Document
{
    int id;
    double relevance;
};


struct SearchQuery
{
    set<string> plus_words ;
    set<string> minus_words ;
};

SearchQuery search_query ;


class SearchServer
{
public:
    void SetStopWords(const string& text)
    {
        for (const string& word : SplitIntoWords(text))
        {
            stop_words_.insert(word);
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const
    {
        const set<string> local_plus_words = ParseQuery(raw_query);

        auto matched_documents = FindAllDocuments(local_plus_words);

        sort(matched_documents.begin(), matched_documents.end(), [](const auto& lhs, const auto& rhs)
                                                                        {
                                                                            return lhs.relevance > rhs.relevance;
                                                                        }                                           );

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
        {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT) ;
        }
        return matched_documents;
    }

    void AddDocument(int document_id, const string& document)
     {
        ++document_count_ ;

        const vector<string> words = SplitIntoWordsNoStop(document);

        for ( const string& word : words)

        {
            word_to_document_freqs_[word][document_id] += ( 1.0 / words.size() ) ;
        }
     }

private:

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_ ;


    int document_count_ = 0;


    double CalcIDF  (string search_query_word ) const
    {
        return ( log ( document_count_ * 1.0 / (word_to_document_freqs_.at(search_query_word).size() ) ) ) ;
    }



    bool IsStopWord(const string& word) const
    {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const
    {
        vector<string> words;
        for (const string& word : SplitIntoWords(text))
        {
            if (!IsStopWord(word))
            {
                words.push_back(word);
            }
        }
        return words;
    }

    set<string> ParseQuery(const string& text) const
    {
        for (const string& current_word : SplitIntoWordsNoStop(text))
        {
            if ( current_word[0] == '-' )
            {
                search_query.minus_words.insert(current_word.substr(1));
            }

            else
            {
                search_query.plus_words.insert(current_word);
            }
        }

        return search_query.plus_words;
    }

    vector<Document> FindAllDocuments(const set<string>& local_plus_words) const
    {
        map<int, double> document_to_relevance ;

        vector<Document> matched_docs ;

        for ( auto& search_query_word : local_plus_words)

        {
            if ( word_to_document_freqs_.count(search_query_word) != 0 )

            {
                for ( auto& [id_docs_for_word , TF ] : word_to_document_freqs_.at(search_query_word) )
                {
                    document_to_relevance[id_docs_for_word] +=  ( TF * CalcIDF(search_query_word) ) ;
                }

            }

        }


        for (const auto& search_query_word : search_query.minus_words )

        {
            if ( word_to_document_freqs_.count(search_query_word) != 0 )

            {
                for ( auto [doc_id, TF] : word_to_document_freqs_.at(search_query_word) )
                {
                    document_to_relevance.erase(doc_id)  ;
                }
            }
        }



        for ( auto [ doc_pos_id, doc_pos_rel ] : document_to_relevance )

        {
            if ( doc_pos_rel ) matched_docs.push_back({doc_pos_id, doc_pos_rel}) ;


        }

        return matched_docs ;
    }
};



SearchServer CreateSearchServer()
{
    SearchServer search_server;

    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();



    for (int document_id = 0; document_id < document_count; ++document_id)
    {
        search_server.AddDocument(document_id, ReadLine());

    }

    return search_server;
}

int main()
{
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();



    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query))
    {
        cout << "{ document_id = "s << document_id << ", " << "relevance = "s << relevance << " }"s << endl  ;
    }
}
