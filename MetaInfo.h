#ifndef METAINFO_H
#define METAINFO_H

#include <string>
#include <vector>
#include <stdlib.h>

using namespace std;

namespace btx
{
    struct MetaFile
    {
        int64_t length;
        string path;
    };

    class MetaInfo
    {
        public:
            MetaInfo(string& meta_info_file);
            ~MetaInfo();

            void parse();

            bool is_single_file();
            const string& get_announce();
            int get_piece_len();
            const string& get_pieces();
            int64_t get_length();
            const string& get_name();
            vector<MetaFile*>& get_meta_files();

        private:
            const static int STATE_STACK_SIZE;
            bool parse_ok_;
            string meta_info_file_;

            string announce_;
            int piece_len_;
            string pieces_;
            bool is_single_file_;
            int64_t length_;
            string name_;
            vector<MetaFile*> meta_files_;
            
            string* prev_key_;
            int state_ptr_;
            MetaInfoParseState state_stack_[STATE_STACK_SIZE];


            void on_s_list(char* buffer, MetaInfoParseState& state, int& buffer_index, int buffer_len);
            void on_s_wait_determin(char* buffer, MetaInfoParseState& state, int& buffer_index, int buffer_len);
            void on_s_string(char* buffer, MetaInfoParseState& state, int& buffer_index, int buffer_len);
            void on_s_int(char* buffer, MetaInfoParseState& state, int& buffer_index, int buffer_len);
            void determin_next_state(MetaInfoParseState& state);
            string* read_str(char* buffer, int& start, int size);
            void skip(buffer_index, num);
            void on_s_dict(char* buffer, MetaInfoParseState& state, int& buffer_index, int buffer_len);
            bool is_dict_end(char* buffer, int buffer_index);
            bool is_list_end(char* buffer, int buffer_index);
            bool is_int_end(char* buffer, int buffer_index);
            bool is_colon(char* buffer, buffer_index);
            void push_state(MetaInfoParseState state);
            MetaInfoParseState pop_prev_state();
            MetaInfoParseState get_prev_state();
            bool is_state_empty();
            int eat_number(char* buffer, int& buffer_index);
            bool is_number(char* buffer, int buffer_index);
    };
}

#endif
