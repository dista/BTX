#include "MetaInfo.h"
#include "BtxException.h"
#include <fstream>
#include <iostream>
#include <string.h>

using namespace btx;

void MetaInfo::parse()
{
    ifstream fs;
    fs.open(meta_info_file_.c_str(), fstream::in | fstream::binary);

    if (fs.fail())
    {
        throw BtxException("open file failed");
    }

    fs.seekg(0, ios::end);
    int file_len = fs.tellg();

    if(file_len > 1024 * 500)
    {
        throw BtxException("torrent file too large, we only support 500KB");
    }

    fs.seekg(0, ios::beg);

    char* buffer = new char[file_len];
    fs.read(buffer, file_len);
    fs.close();

    int idx = 0;

    MetaInfoParseState state;
    state = MI_WAIT_DETERMIN; 

    while(idx < file_len)
    {
        switch(state)
        {
            case MI_WAIT_DETERMIN:
               on_s_wait_determin(buffer, state, idx, file_len);
               break;
            case MI_DICT_WAIT_KEY:
            case MI_DICT_WAIT_VALUE:
               on_s_dict(buffer, state, idx, file_len);
               break;
            case MI_LIST:
               on_s_list(buffer, state, idx, file_len); 
               break;
            case MI_STRING:
               on_s_string(buffer, state, idx, file_len);
               break;
            case MI_INT:
               on_s_int(buffer, state, idx, file_len);
               break;
        }
    }
}

void MetaInfo::on_s_list(char* buffer, MetaInfoParseState& state, int& buffer_index, int buffer_len)
{
    if(is_list_end(buffer, buffer_index))
    {
        state = pop_prev_state();
        skip(buffer_index, 1);
        return;
    }
    else
    {
        on_s_wait_determin(buffer, state, buffer_index, buffer_len);
    }
}

void MetaInfo::on_s_wait_determin(char* buffer, MetaInfoParseState& state, int& buffer_index, int buffer_len)
{
    push_state(state);
    int c = (int)buffer[buffer_index];

    switch(c)
    {
        case (int)'d':
            state = MI_DICT_WAIT_KEY;
            buffer_index++;
            break;
        case (int)'i':
            state = MI_INT;
            break;
        case (int)'l':
            state = MI_LIST;
            break;
        default:
            if(is_number(buffer, buffer_index))
            {
                state = MI_STRING;
            }
            else
            {
                throw BtxException("unknown type");
            }
    }

    if(state == MI_DICT_WAIT_KEY || state == MI_INT || state == MI_LIST)
    {
        skip(buffer_index, 1);
    }
}

void MetaInfo::on_s_string(char* buffer, MetaInfoParseState& state, int& buffer_index, int buffer_len)
{
    int str_len = eat_number(buffer, buffer_index);

    if(!is_colon(buffer, buffer_index))
    {
        throw BtxException("parse string error");
    }

    skip(buffer_index, 1);

    if(buffer_index + str_len > buffer_len)
    {
        throw BtxException("bad string length");
    }

    if(prev_key_)
    {
        delete prev_key_;
    }

    prev_key_ = read_str(buffer, buffer_index, str_len);

    determin_next_state(state);
}

void MetaInfo::on_s_int(char* buffer, MetaInfoParseState& state, int& buffer_index, int buffer_len)
{
    if(!is_number(buffer, buffer_index))
    {
        throw BtxException("expect number");
    }

    int num = eat_number(buffer, buffer_index);

    if(!is_int_end(buffer, buffer_index))
    {
        throw BtxException("expect number end");
    }

    skip(buffer_index, 1);

    determin_next_state(state);
}

void MetaInfo::determin_next_state(MetaInfoParseState& state)
{
    MetaInfoParseState prev_state = pop_prev_state();

    switch(prev_state)
    {
        case MI_DICT_WAIT_KEY:
            state = MI_DICT_WAIT_VALUE;
            break;
        case MI_DICT_WAIT_VALUE:
            state = MI_DICT_WAIT_KEY;
            break;
        default:
            state = prev_state;
    }
}

string* MetaInfo::read_str(char* buffer, int& start, int size)
{
    char* buf = new char[size + 1];
    buf[size] = 0;
    memcpy(buf, buffer + start, size);

    string* s = new string(buf);
    delete buf;

    start += size;
    return s;
}

void MetaInfo::skip(int& buffer_index, int num)
{
    buffer_index += num;
}

void MetaInfo::on_s_dict(char* buffer, MetaInfoParseState& state, int& buffer_index, int buffer_len)
{
    if(state == MI_DICT_WAIT_KEY)
    {
        if(is_dict_end(buffer, buffer_index))
        {
            skip(buffer_index, 1);
            state = pop_prev_state();
            return;
        }

        if(!is_number(buffer, buffer_index))
        {
            throw BtxException("bad format, can't find key for dict");
        }

        push_state(state);
        state = MI_STRING;
    }
    else
    {
        on_s_wait_determin(buffer, state, buffer_index, buffer_len);
    }
}

bool MetaInfo::is_dict_end(char* buffer, int buffer_index)
{
    return buffer[buffer_index] == 'e';
}

bool MetaInfo::is_list_end(char* buffer, int buffer_index)
{
    return buffer[buffer_index] == 'e';
}

bool MetaInfo::is_int_end(char* buffer, int buffer_index)
{
    return buffer[buffer_index] == 'e';
}

bool MetaInfo::is_colon(char* buffer, int buffer_index)
{
    if(buffer[buffer_index] == ':')
    {
        return true;
    }

    return false;
}

void MetaInfo::push_state(MetaInfoParseState state)
{
    if(state_ptr_ == MetaInfo::STATE_STACK_SIZE)
    {
        throw BtxException("nested too much");
    }

    state_stack_[state_ptr_++] = state;
}

MetaInfoParseState MetaInfo::pop_prev_state()
{
    if(state_ptr_ == 0)
    {
        throw BtxException("state_stack_ is empty");
    }

    return state_stack_[state_ptr_--];
}

MetaInfoParseState MetaInfo::get_prev_state()
{
    if(state_ptr_ == 0)
    {
        throw BtxException("state_stack_ is empty");
    }

    return state_stack_[state_ptr_ - 1];
}

bool MetaInfo::is_state_empty()
{
    return state_ptr_ == 0;
}

int MetaInfo::eat_number(char* buffer, int& buffer_index)
{
    int ret = 0;

    while(is_number(buffer, buffer_index))
    {
        ret = 10 * ret + (int)buffer[buffer_index];
        buffer_index++;
    }

    return ret;
}

bool MetaInfo::is_number(char* buffer, int buffer_index)
{
    if(buffer[buffer_index] >= '0' && buffer[buffer_index] <= '9')
    {
        return true;
    }

    return false;
}

bool MetaInfo::is_single_file()
{
    return is_single_file_;
}

const string& MetaInfo::get_announce()
{
    return announce_;
}

int MetaInfo::get_piece_len()
{
    return piece_len_;
}

const string& MetaInfo::get_pieces()
{
    return pieces_;
}

const string& MetaInfo::get_name()
{
    return name_;
}

vector<MetaFile*>& MetaInfo::get_meta_files()
{
    return meta_files_;
}

MetaInfo::MetaInfo(string& meta_info_file)
    :parse_ok_(false),
    length_(0),
    is_single_file_(true),
    piece_len_(0),
    prev_key_(NULL),
    state_ptr_(0),
    meta_info_file_(meta_info_file)
{
}

MetaInfo::~MetaInfo()
{
    vector<MetaFile*>::iterator it;

    for(it = meta_files_.begin(); it < meta_files_.end(); it++)
    {
        delete *it;
    }

    meta_files_.clear();

    if(prev_key_ != NULL)
    {
        delete prev_key_;
    }
}
