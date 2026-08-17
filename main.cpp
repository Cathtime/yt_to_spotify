#include <cpr/cpr.h>
#include <iostream>
#include <lexbor/html/html.h>
#include <lexbor/css/css.h>
#include <lexbor/html/node.h>
#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>
#include <lexbor/selectors/selectors.h>
#include <cstdio>
#include <cstring>

lxb_status_t callback(lxb_dom_node_t* node, lxb_css_selector_specificity_t spec, void *ctx) {
    std::cout << "Found!\n";

    std::cout << node->local_name;

    return LXB_STATUS_OK;
}

lxb_status_t print_callback(const lxb_char_t *data, size_t len, void *ctx) {
    // Cast data safely to const char* and use std::cout to print it
    std::cout << std::string_view(reinterpret_cast<const char*>(data), len) << std::endl;
    return LXB_STATUS_OK;
}


int main(int argc, char* argv[]) {

    cpr::Response r = cpr::Get(cpr::Url{"https://www.youtube.com/playlist?list=PLTbHatCdejA_ZaO-y7oTOMDEViOhW93Bd"});

   // std::cout << "Response body: " << r.text << std::endl;

   // okay so the response gets the html
   // now lets see if lexbor gets the html too (how its supposed to smh)

    const lxb_char_t* html = reinterpret_cast<const lxb_char_t*>(r.text.c_str());

    lxb_html_document_t* document = lxb_html_document_create();
    lxb_status_t status = lxb_html_document_parse(document,html, r.text.size());

    if (status != LXB_STATUS_OK) {
        std::cerr << "Failed to parse HTML document.\n" ;
        return 1;
    } else {
      //  lxb_html_serialize_deep_cb(&document->head->element.element.node, print_callback, nullptr);
      // lxb_html_serialize_deep_cb(&document->body->element.element.node, print_callback, nullptr);  
    }

    lxb_selectors_t* selectors = lxb_selectors_create();

    lxb_status_t initSelec = lxb_selectors_init(selectors);

    if (initSelec != LXB_STATUS_OK) {
        std::cerr << "Selector failed to initialize\n";
    }

    lxb_css_parser_t* parser =  lxb_css_parser_create();
    lxb_css_parser_init(parser, NULL);

    // starts off as regular string
    std::string selector = "watchEndpoint";

    // convert it to lexbors version of char pointer
    const lxb_char_t* lexbor_string = reinterpret_cast<const lxb_char_t*>(selector.c_str());

    size_t length = selector.size(); // std::string knows its size so we use this 

    lxb_css_selector_list_t *list = lxb_css_selectors_parse(parser, lexbor_string, length);

    if (parser->status != LXB_STATUS_OK) {
        std::cout << "Failed\n";
        std::cerr << parser->status << std::endl;
    } else {
        std:: cout << "List printed out: \n";
 //       lxb_css_selector_serialize_list(list, print_callback, nullptr);
    }

    
/*
    lxb_status_t foundClass = lxb_selectors_find(
        selectors, 
        &document->head->element.element.node, 
        list, 
        callback,
        nullptr
    );

    if (foundClass != LXB_STATUS_OK) {
        std::cout << "Something went wrong here smh\n";
        std::cerr << foundClass << std::endl;
    }
*/
    lxb_status_t foundClass = lxb_selectors_find(
        selectors, 
        &document->body->element.element.node, 
        list, 
        callback,
        nullptr
    );

    if (foundClass != LXB_STATUS_OK) {
        std::cout << "Something went wrong here smh\n";
        std::cerr << foundClass << std::endl;
    } else {
        std:: cout << "List printed out: \n";
       // lxb_css_selector_serialize_list(list, print_callback, nullptr);
    }

   // document->body->element.element.node.first_child;    

    return 0;
}