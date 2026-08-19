#include <cpr/cpr.h>
#include <iostream>
#include <lexbor/html/html.h>
#include <lexbor/html/serialize.h>
#include <lexbor/html/serialize.h>
#include <lexbor/css/css.h>
#include <lexbor/html/node.h>
#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>
#include <lexbor/selectors/selectors.h>
#include <cstdio>
#include <cstring>

// need to find a way to seperate callback and printcallback
// I need to extract information for the video links too but I can't just pass them like that because obviously its built for the playlist
// Maybe I could have flags? probably some enums inside (I would just pass different struct context, 
// but c++ void pointers do NOT implicitly change the type like in c so kys c++)

// using an enum to correctly seperate playlist data from video data
// even if it makes my life worse some how but thats what I get for working with C libraries smh
enum class Type {
    PLAYLIST,
    SIGNATURE,
    BASE
};

// for the playlist, since we're seperating INSIDE THE FUNCTION (although maybe I'll do a helper function to reduce code dupe)
// I need to get everything relevant to the playlist we need
// video id size, the anchor, etc
struct PlaylistData {
    static constexpr std::string_view anchor = "\"watchEndpoint\":{\"videoId\":\"";
    static constexpr char ending = '"';
    static constexpr size_t MAX_TAIL = 32 * 1024;
};

// next up "SignatureCipher"
// (help)
struct SignatureData {
    static constexpr std::string_view anchor = "\"watchEndpoint\":{\"videoId\":\"";
    static constexpr char ending = '"';
    static constexpr size_t MAX_TAIL = 32 * 1024;
};

struct BaseData {
    static constexpr std::string_view anchor = "\"PLAYER_JS_URL\":\"";
    static constexpr char ending = '"';
    static constexpr size_t MAX_TAIL = 32 * 1024;
};

struct SerializeContext {
    std::string tail;
    std::set<std::string> element_values;
    Type type;    
};

// okay so this is a lot of bullshit in here but there's nothing I can do unless there is but we'll see later
// to try and make everything MAKE SENSE and not have to have like 5 different callbacks for something that can be simplified
// I'm going to use SerializeContext as the thing that brings everything else together
// the other structs just hold the data for each case

static bool is_valid_youtube_id(const std::string& s) {
    if (s.size() != 11) return false;
    for (unsigned char ch : s) {
        if (!(std::isalnum(ch) || ch == '_' || ch == '-')) return false;
    }
    return true;
}

template <typename T>
void node_string_manip(const lxb_char_t* data, size_t len, SerializeContext* sctx, const T& sData) {
     sctx->tail.append(reinterpret_cast<const char*>(data), len);

    auto anchor = sData.anchor;
    auto MAX_TAIL = sData.MAX_TAIL;
    auto ending = sData.ending;

    size_t search_pos = 0;
    while (true) {
        size_t p = sctx->tail.find(anchor, search_pos);
        if (p == std::string::npos) break;

        size_t id_start = p + anchor.size();

        size_t id_end = sctx->tail.find(ending, id_start);

        if (id_end == std::string::npos) {
            break;
        }

        std::string candidate = sctx->tail.substr(id_start, id_end - id_start);
        if (sctx->type == Type::PLAYLIST && is_valid_youtube_id(candidate)) {
            sctx->element_values.insert(candidate);
        }

        sctx->element_values.insert(candidate);  

        search_pos = id_end + 1;
    }

    if (sctx->tail.size() > MAX_TAIL) {
        sctx->tail.erase(0, sctx->tail.size() - MAX_TAIL);
    }
}

lxb_status_t print_callback(const lxb_char_t* data, size_t len, void* ctx) {
    auto* sctx = static_cast<SerializeContext*>(ctx);
    if (sctx == nullptr || data == nullptr || len == 0) {
        return LXB_STATUS_OK;
    }

    switch(sctx->type) {
        case Type::BASE:
            BaseData Bdata;

            node_string_manip(data,len,sctx,Bdata);

            break;

        case Type::PLAYLIST:
            PlaylistData Pdata;

            node_string_manip(data,len,sctx,Pdata);

            break;
        
        case Type::SIGNATURE:
            break;
    }
   
    return LXB_STATUS_OK;
}

lexbor_action_t callback(lxb_dom_node_t* node, void* ctx) {

    auto* sctx = static_cast<SerializeContext*>(ctx);

    lxb_html_serialize_cb(node, print_callback, sctx);

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

    SerializeContext playlistctx;

    if (status != LXB_STATUS_OK) {
        std::cerr << "Failed to parse HTML document.\n" ;
        return 1;
    } else {
      //  lxb_html_serialize_deep_cb(&document->head->element.element.node, print_callback, nullptr);
      // lxb_html_serialize_deep_cb(&document->body->element.element.node, print_callback, nullptr);  
    }

    playlistctx.type = Type::PLAYLIST;

    lxb_dom_node_simple_walk(&document->body->element.element.node, callback, &playlistctx);

    for (const auto& id : playlistctx.element_values) {
        std::cout << "https://www.youtube.com/watch?v=" << id << "\n";
    }

    // base.js is located in the head so I have to run this again
    SerializeContext Basectx;

    Basectx.type = Type::BASE;

    lxb_dom_node_simple_walk(&document->head->element.element.node, callback, &Basectx);

    for (const auto& id : Basectx.element_values) {
        std::cout << id << "\n";
    }

   // document->body->element.element.node.first_child;   
   
   lxb_html_document_destroy(document);

    return 0;
}