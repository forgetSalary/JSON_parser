int indent = 0;

void print_newline(void) {
    printf("\n%.*s", 2*indent, "                                                                               ");
}

void json_print_object(JsonObject* object);
char* json_stringify_object(BUF(char* buffer),JsonObject* object);

void json_print_value(JsonValue* value){
    assert(value);
    switch (value->type) {
        case JSON_number:
            printf("%f",value->number);
            break;
        case JSON_string:
            printf("\"%s\"",value->string.str);
            break;
        case JSON_array:
            printf("[");
            if(value->array.len){
                for (JsonValue** it = value->array.values; it != value->array.values + (value->array.len - 1); it++){
                    json_print_value(*it);
                    printf(",");
                }
                json_print_value(value->array.values[value->array.len - 1]);
            }
            printf("]");
            break;
        case JSON_bool:
            if (value->boolean){
                printf("true");
            }else{
                printf("false");
            }
            break;
        case JSON_null:
            printf("null");
            break;
        case JSON_object:
            json_print_object(value->object);
            break;
    }
}

void json_print_field(JsonField* field){
    assert(field);
    printf("\"%s\": ",field->key);
    json_print_value(field->value);
}

void json_print_object(JsonObject* object){
    assert(object);
    printf("{");
    if (object->fields_count){
        indent++;
        for (JsonField** field = object->fields; field != object->fields + (object->fields_count-1); field++){
            print_newline();
            json_print_field(*field);
            printf(",");
        }
        print_newline();
        json_print_field(object->fields[object->fields_count-1]);
        indent--;
        print_newline();
    }
    printf("}");
}
#define json_printf json_print_object

char* json_stringify_value(char* buffer, JsonValue* value){
    assert(value);
    switch (value->type) {
        case JSON_number:
            buf_printf(buffer,"%f",value->number);
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

char* json_stringify_field(char* buffer,JsonField* field){
    assert(field);
    buf_printf(buffer,"\"%s\":",field->key);
    return json_stringify_value(buffer,field->value);
}

char* json_stringify_object(BUF(char* buffer),JsonObject* object){
    assert(object);
    buf_printf(buffer,"{");
    if (object->fields_count){
        for (JsonField** field = object->fields; field != object->fields + (object->fields_count-1); field++){
            buffer = json_stringify_field(buffer,*field);
            buf_printf(buffer,",");
        }
        buffer = json_stringify_field(buffer,object->fields[object->fields_count-1]);
    }
    buf_printf(buffer,"}");
    return buffer;
}

#define json_stringify(object) json_stringify_object(NULL,object)

void json_print_test(){
    JsonObject* obj = json_object((JsonField*[]){
        json_field("Number", json_value_number(3.14)),
        json_field("String",json_value_string(json_string("hello"))),
        json_field("Boolean", json_value_boolean(true)),
        json_field("Null", json_value_null()),
        json_field("Array", json_value_array(
                (JsonValue *[]) {json_value_number(1), json_value_number(2), json_value_number(3)}, 3)),
        json_field("Child-Object",json_value_object(json_object((JsonField*[]){
                json_field("Number", json_value_number(123)),
                json_field("String",json_value_string(json_string("foobar"))),
                json_field("Boolean", json_value_boolean(false))
                },3)))
        },6);
    json_printf(obj);

    json_get_field(obj,"String")->value->string = json_string("world");
    json_get_field(json_get_field(obj,"Child-Object")->value->object,"Number")->value->number = 321;
    json_put_field(obj,json_field("Empty-Object",json_value_object(NULL)));
    json_put_field(obj,json_field("Empty-Array", json_value_array(NULL, 0)));
    printf("\n\n");
    json_print_object(obj);

    char* obj_str = json_stringify(obj);
    printf("\n\n%s",obj_str);
    buf_free(obj_str);

    free_json_data();
}