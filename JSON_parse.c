
static JsonObject* json_parse_object();

static JsonValue* json_parse_value(){
    JsonValue* new_value = NULL;
    switch (token.kind) {
        case TOKEN_STR:
            new_value = json_value_string((JsonString){token.str_val,buf_len(token.str_val)});
            break;
        case TOKEN_INT:
            new_value = json_value_number_int(token.int_val);
            break;
        case TOKEN_FLOAT:
            new_value = json_value_number_float(token.float_val);
            break;
        case TOKEN_NAME:
            if (is_keyword_str(token.name)){
                if (token.name == true_keyword){
                    new_value = json_value_boolean( 1);
                }else if(token.name == false_keyword){
                    new_value = json_value_boolean( 0);
                }else if(token.name == null_keyword){
                    new_value = json_value_null();
                }
            }else{
                fatal("Unexpected name token");
            }
            break;
        case '[':
            new_value = json_value_array(NULL,0);
            next_token();
            if(!is_token(']')){
                for(;;){
                    json_array_push(&new_value->array,json_parse_value());
                    if (!is_token(',')){
                        if (is_token(']')){
                            break;
                        }else{
                            expect_token(']');
                        }
                    }
                    next_token();
                }
            }
            break;
        case '{':
            next_token();
            new_value = json_value_object(json_parse_object());
            break;
    }
    assert(new_value);
    next_token();
    return new_value;
}

static JsonField* json_parse_field(){
    const char* key = token.str_val;
    next_token();
    if(expect_token(':')){
        return json_field(key,json_parse_value());
    } else{
        return NULL;
    }
}

static JsonObject* json_parse_object(){
    JsonObject* object = json_object(NULL,0);
    while (1){
        if (is_token(TOKEN_STR)){
            json_put_field(object,json_parse_field());
            if (is_token('}')){
                break;
            }else{
                expect_token(',');
            }
        }else if(expect_token('}')){
            return object;
        }
    }
    return object;
}

JsonObject* json_parse(char* str){
    init_stream(str);
    init_keywords();
    if (match_token('{')){
        return json_parse_object();
    }
    arena_free(&str_arena);
    free(interns.entries);
    return NULL;
}
