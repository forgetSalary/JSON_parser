
void json_parse_test(){
    JsonObject* obj1 = json_parse("{}");
    JsonObject* obj2 = json_parse(read_file("./test.json"));

    json_fprintf(stdout,obj1);
    printf("\n\n");
    json_fprintf(stdout,obj2);

}


void json_print_test(){
    JsonObject* obj = json_object((JsonField*[]){
            json_field("Number", json_value_number_float(3.14)),
            json_field("String",json_value_string(json_string("hello"))),
            json_field("Boolean", json_value_boolean(true)),
            json_field("Null", json_value_null()),
            json_field("Array", json_value_array(
                    (JsonValue *[]) {json_value_number_int(1), json_value_number_int(2), json_value_number_int(3)}, 3)),
            json_field("Child-Object",json_value_object(json_object((JsonField*[]){
                    json_field("Number", json_value_number_int(123)),
                    json_field("String",json_value_string(json_string("foobar"))),
                    json_field("Boolean", json_value_boolean(false))
            },3)))
    },6);
    json_fprintf(stdout,obj,true);

    json_get_field(obj,"String")->value->string = json_string("world");
    json_get_field(json_get_field(obj,"Child-Object")->value->object,"Number")->value->float_number = 321;
    json_put_field(obj,json_field("Empty-Object",json_value_object(NULL)));
    json_put_field(obj,json_field("Empty-Array", json_value_array(NULL, 0)));
    printf("\n\n");
    json_fprintf(stdout,obj,true);

    char* obj_str = json_stringify(obj,false);
    printf("\n\n%s",obj_str);
    buf_free(obj_str);

    free_json_data();
}