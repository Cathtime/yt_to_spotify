#include <cpr/cpr.h>
#include <iostream>
#include <lexbor/html/html.h>
#include <lexbor/html/serialize.h>
#include <lexbor/css/css.h>
#include <lexbor/html/node.h>
#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>
#include <lexbor/selectors/selectors.h>
#include <cstdio>
#include <cstring>

struct SerializeContext {
    std::string tail;
    std::set<std::string> video_ids;    
};

static bool is_valid_youtube_id(const std::string& s) {
    if (s.size() != 11) return false;
    for (unsigned char ch : s) {
        if (!(std::isalnum(ch) || ch == '_' || ch == '-')) return false;
    }
    return true;
}


lxb_status_t print_callback(const lxb_char_t* data, size_t len, void* ctx) {
    auto* sctx = static_cast<SerializeContext*>(ctx);
    if (sctx == nullptr || data == nullptr || len == 0) {
        return LXB_STATUS_OK;
    }

    static const std::string anchor = "\"watchEndpoint\":{\"videoId\":\"";
    static constexpr size_t MAX_TAIL = 32 * 1024; // keep memory bounded

    // save what we're receiving to the tail 
    sctx->tail.append(reinterpret_cast<const char*>(data), len);

    std::cout << "size tail: " << sctx->tail.size() << "\n";

    size_t search_pos = 0;
    while (true) {
        size_t p = sctx->tail.find(anchor, search_pos);
        if (p == std::string::npos) break;

        std::cout << "p: " << p << "\n";

        size_t id_start = p + anchor.size();

        std::cout << "id_start: " << id_start << "\n";

        size_t id_end = sctx->tail.find('"', id_start);

        if (id_end == std::string::npos) {
            break;
        }

      //  std::cout << "tail: " << sctx->tail << std::endl;

        std::string candidate = sctx->tail.substr(id_start, id_end - id_start);
        if (is_valid_youtube_id(candidate)) {
            sctx->video_ids.insert(candidate);
        }

        search_pos = id_end + 1;
    }

    if (sctx->tail.size() > MAX_TAIL) {
        sctx->tail.erase(0, sctx->tail.size() - MAX_TAIL);

        //std::cout << "Current tail: " << sctx->tail << std::endl;
    }

    return LXB_STATUS_OK;
}

lexbor_action_t callback(lxb_dom_node_t* node, void* ctx) {
    SerializeContext sctx;

    lxb_html_serialize_cb(node, print_callback, &sctx);

    std::cout << "tail: \n" << sctx.tail << "\n";

    for (const auto& id : sctx.video_ids) {
        std::cout << "https://www.youtube.com/watch?v=" << id << "\n";
    }

    return LEXBOR_ACTION_OK;
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

    lxb_dom_node_simple_walk(&document->body->element.element.node, callback, nullptr);


   // document->body->element.element.node.first_child;   
   
   lxb_html_document_destroy(document);

    return 0;
}