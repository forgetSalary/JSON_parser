int indent = 0;

#define buf_printf_newline(buf)\
buf_printf(buf,"\n%.*s", 2*indent, "                                                                               ")

char* json_stringify_object(BUF(char* buffer),JsonObject* object);

static char* json_stringify_value(char* buffer, JsonValue* value){
    assert(value);
    switch (value->type) {
        case JSON_number_float:
            buf_printf(buffer,"%f",value->float_number);
            break;
        case JSON_number_int:
            buf_printf(buffer,"%d",value->int_number);
            break;
        case JSON_string:
            buf_printf(buffer,"\"%s\"",value->string.str);
            break;
        case JSON_array:
            buf_printf(buffer,"[");
            if(value->array.len){
                for (JsonValue** it = value->array.values; it != value->array.values + (value->array.len - 1); it++){
                    buffer = json_stringify_value(buffer,*it);
                    buf_printf(buffer,",");
                }
                buffer = json_stringify_value(buffer,value->array.values[value->array.len - 1]);
            }
            buf_printf(buffer,"]");
            break;
        case JSON_bool:
            if (value->boolean){
                buf_printf(buffer,"true");
            }else{
                buf_printf(buffer,"false");
            }
            break;
        case JSON_null:
            buf_printf(buffer,"null");
            break;
        case JSON_object:
            buffer = json_stringify_object(buffer,value->object);
            break;
    }
    return buffer;
}

static char* json_stringify_field(char* buffer,JsonField* field){
    assert(field);
    buf_printf(buffer,"\"%s\":",field->key);
    return json_stringify_value(buffer,field->value);
}

char* json_stringify_object(BUF(char* buffer),JsonObject* object){
    assert(object);
    object->format_print ? buf_printf_newline(buffer) : 0;
    buf_printf(buffer,"{");
    if (object->fields_count){
        object->format_print ? indent++ : 0;
        for (JsonField** field = object->fields; field != object->fields + (object->fields_count-1); field++){
            object->format_print ? buf_printf_newline(buffer) : 0;
            buffer = json_stringify_field(buffer,*field);
            buf_printf(buffer,",");
        }
        object->format_print ? buf_printf_newline(buffer) : 0;
        buffer = json_stringify_field(buffer,object->fields[object->fields_count-1]);
        object->format_print ? indent-- : 0;
    }
    object->format_print ? buf_printf_newline(buffer) : 0;
    buf_printf(buffer,"}");
    return buffer;
}

char* json_stringify(JsonObject* obj){
    char* buffer = json_stringify_object(NULL,obj);
    char* returned = strdup(buffer);
    buf_free(buffer);
    return returned;
}

void json_fprintf(FILE* stream,JsonObject* obj){
    char* buffer = json_stringify_object(NULL,obj);
    fprintf(stream,buffer);
    buf_free(buffer);
}
