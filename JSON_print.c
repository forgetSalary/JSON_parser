int indent = 0;

void print_newline(void) {
    printf("\n%.*s", 2*indent, "                                                                               ");
}

void json_print_object(JsonObject* object);

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
    json_print_object(obj);

    json_get_field(obj,"String")->value->string = json_string("world");
    json_get_field(json_get_field(obj,"Child-Object")->value->object,"Number")->value->number = 321;
    json_put_field(obj,json_field("Empty-Object",json_value_object(NULL)));
    json_put_field(obj,json_field("Empty-Array", json_value_array(NULL, 0)));
    printf("\n\n");
    json_print_object(obj);

    free_json_data();
}