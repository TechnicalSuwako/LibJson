#include <cassert>
#include <iostream>
#include <suwa/json.hh>
#include <util/file.hh>

int main(void) {
  {
    const string src = R"({
      "name": "諏訪子",
      "kero": "🐸",
      "age": 2300,
      "alive": true,
      "power": 99.5,
      "items": ["hat", "frog", 42],
      "nested": {
        "faith": null
      }
    })";

    auto res = json::Parser(src).parse();
    assert(res.error == json::Error::None);
    assert(res.value);
    assert(res.value->is_object());

    auto &root = res.value->as_object();
    assert(root.get("name")->as_string() == "諏訪子");
    assert(root.get("kero")->as_string() == "🐸");
    assert(root.get("age")->as_number() == 2300);
    assert(root.get("alive")->as_bool() == true);
    assert(root.get("power")->as_number() == 99.5);

    auto &items = root.get("items")->as_array();
    assert(items.size() == 3);
    assert(items[0].as_string() == "hat");
    assert(items[1].as_string() == "frog");
    assert(items[2].as_number() == 42);

    auto &nested = root.get("nested")->as_object();
    assert(nested.get("faith")->is_null());

    auto out = res.value->serialize();
    std::cout << out << std::endl;
    auto out2 = res.value->serialize({ .pretty = true, .indent = 2 });
    std::cout << out2 << std::endl;
    auto out3 = res.value->serialize({ .pretty = true, .indent = 8 });
    std::cout << out3 << std::endl;

    auto res2 = json::Parser(out).parse();
    assert(res2.error == json::Error::None);

    auto res3 = json::Parser(out2).parse();
    assert(res3.error == json::Error::None);

    auto res4 = json::Parser(out3).parse();
    assert(res4.error == json::Error::None);

    std::cout << "Memory OK" << std::endl;
  }

  {
    auto res = json::Parser::parse_from_file("webfinger.json");
    assert(res.error == json::Error::None);
    assert(res.value);
    assert(res.value->is_object());

    auto &root = res.value->as_object();
    auto *links = root.get("links");
    assert(links);
    assert(links->is_array());

    auto &arr = links->as_array();
    assert(arr.size() == 1);
    assert(arr[0].is_object());
    auto &link = arr[0].as_object();

    assert(link.get("href"));
    assert(link.get("rel"));
    assert(link.get("type"));

    assert(link.get("href")->as_string() == "https://sns.076.moe/users/suwako");
    assert(link.get("rel")->as_string() == "self");
    assert(link.get("type")->as_string() == "application/activity+json");

    auto *subject = root.get("subject");
    assert(subject);
    assert(subject->as_string() == "acct:suwako@sns.076.moe");

    auto out = res.value->serialize_to_file("webfinger2.json");
    auto out2 = res.value->serialize_to_file("webfinger3.json", { .pretty = true, .indent = 2 });

    auto res2 = json::Parser::parse_from_file("webfinger2.json");
    assert(res2.error == json::Error::None);

    auto res3 = json::Parser::parse_from_file("webfinger3.json");
    assert(res3.error == json::Error::None);

    std::cout << "WebFinger OK" << std::endl;
  }

  {
    auto res = json::Parser::parse_from_file("gltf.json");
    assert(res.error == json::Error::None);
    assert(res.value);
    assert(res.value->is_object());

    auto &root = res.value->as_object();
    assert(root.get("scene")->as_number() == 0);

    {
      auto *scenes = root.get("scenes");
      assert(scenes);
      assert(scenes->is_array());

      auto &arr = scenes->as_array();
      assert(arr.size() == 1);
      assert(arr[0].is_object());
      auto &scene = arr[0].as_object();

      assert(scene.get("nodes")->is_array());
      auto &sceneNodes = scene.get("nodes")->as_array();
      assert(sceneNodes.size() == 1);
      assert(sceneNodes[0].as_number() == 0);
    }

    {
      auto *nodes = root.get("nodes");
      auto &arr = nodes->as_array();
      assert(arr.size() == 1);
      assert(arr[0].is_object());
      auto &node = arr[0].as_object();

      assert(node.get("mesh")->as_number() == 0);
    }

    {
      auto *buffers = root.get("buffers");
      auto &arr = buffers->as_array();
      assert(arr.size() == 1);
      assert(arr[0].is_object());
      assert(arr[0].as_object().get("uri")->as_string() == "data:application/octet-stream;base64,AAABAAIAAAAAAAAAAAAAAAAAAAAAAIA/AAAAAAAAAAAAAAAAAACAPwAAAAA=");
      assert(arr[0].as_object().get("byteLength")->as_number() == 44);
    }

    {
      auto *bufferViews = root.get("bufferViews");
      auto &arr = bufferViews->as_array();
      assert(arr.size() == 2);
      assert(arr[0].is_object());
      assert(arr[0].as_object().get("buffer")->as_number() == 0);
      assert(arr[0].as_object().get("byteOffset")->as_number() == 0);
      assert(arr[0].as_object().get("byteLength")->as_number() == 6);
      assert(arr[0].as_object().get("target")->as_number() == 34963);

      assert(arr[1].as_object().get("buffer")->as_number() == 0);
      assert(arr[1].as_object().get("byteOffset")->as_number() == 8);
      assert(arr[1].as_object().get("byteLength")->as_number() == 36);
      assert(arr[1].as_object().get("target")->as_number() == 34962);
    }

    {
      auto *accessors = root.get("accessors");
      auto &arr = accessors->as_array();
      assert(arr.size() == 2);
      assert(arr[0].is_object());
      assert(arr[0].as_object().get("bufferView")->as_number() == 0);
      assert(arr[0].as_object().get("byteOffset")->as_number() == 0);
      assert(arr[0].as_object().get("componentType")->as_number() == 5123);
      assert(arr[0].as_object().get("count")->as_number() == 3);
      assert(arr[0].as_object().get("type")->as_string() == "SCALAR");
      auto max0 = arr[0].as_object().get("max")->as_array();
      assert(max0.size() == 1);
      assert(max0[0].get_number() == 2);
      auto min0 = arr[0].as_object().get("min")->as_array();
      assert(min0.size() == 1);
      assert(min0[0].get_number() == 0);

      assert(arr[1].is_object());
      assert(arr[1].as_object().get("bufferView")->as_number() == 1);
      assert(arr[1].as_object().get("byteOffset")->as_number() == 0);
      assert(arr[1].as_object().get("componentType")->as_number() == 5126);
      assert(arr[1].as_object().get("count")->as_number() == 3);
      assert(arr[1].as_object().get("type")->as_string() == "VEC3");
      auto max1 = arr[1].as_object().get("max")->as_array();
      assert(max1.size() == 3);
      assert(max1[0].get_number() == 1.0);
      assert(max1[1].get_number() == 1.0);
      assert(max1[2].get_number() == 0.0);
      auto min1 = arr[1].as_object().get("min")->as_array();
      assert(min1.size() == 3);
      assert(min1[0].get_number() == 0.0);
      assert(min1[1].get_number() == 0.0);
      assert(min1[2].get_number() == 0.0);
    }

    {
      auto *asset = root.get("asset");
      auto &obj = asset->as_object();
      assert(obj.get("version")->as_string() == "2.0");
    }

    auto out = res.value->serialize_to_file("gltf2.json");
    auto out2 = res.value->serialize_to_file("gltf3.json", {.pretty = true, .indent = 2});

    auto res2 = json::Parser::parse_from_file("gltf2.json");
    assert(res2.error == json::Error::None);

    auto res3 = json::Parser::parse_from_file("gltf3.json");
    assert(res3.error == json::Error::None);

    std::cout << "glTF OK" << std::endl;
  }

  {
    auto res = json::Parser::parse_from_file("activitypub.json");
    assert(res.error == json::Error::None);
    assert(res.value);
    assert(res.value->is_object());

    auto &root = res.value->as_object();
    auto *context = root.get("@context");
    assert(context);
    assert(context->is_array());

    auto &arr = context->as_array();
    assert(arr.size() == 2);
    assert(arr[0].is_string());
    assert(arr[1].is_string());
    assert(arr[0].get_string() == "https://www.w3.org/ns/activitystreams");
    assert(arr[1].get_string() == "https://w3id.org/security/v1");

    auto *type = root.get("type");
    assert(type);
    assert(type->as_string() == "Person");

    auto *id = root.get("id");
    assert(id);
    assert(id->as_string() == "https://example.social/users/sam");

    auto *preferredUsername = root.get("preferredUsername");
    assert(preferredUsername);
    assert(preferredUsername->as_string() == "sam");

    auto *inbox = root.get("inbox");
    assert(inbox);
    assert(inbox->as_string() == "https://example.social/inbox");

    auto *publicKey = root.get("publicKey");
    assert(publicKey);
    assert(publicKey->is_object());

    auto &pubkey = publicKey->as_object();
    assert(pubkey.get("id")->as_string() == "https://example.social/users/sam#main-key");
    assert(pubkey.get("owner")->as_string() == "https://example.social/users/sam");
    assert(pubkey.get("publicKeyPem")->as_string() == "-----BEGIN PUBLIC KEY-----\n...\n----END PUBLIC KEY----");

    auto out = res.value->serialize_to_file("activitypub2.json");
    auto out2 = res.value->serialize_to_file("activitypub3.json", { .pretty = true, .indent = 2 });

    auto res2 = json::Parser::parse_from_file("activitypub2.json");
    assert(res2.error == json::Error::None);

    auto res3 = json::Parser::parse_from_file("activitypub3.json");
    assert(res3.error == json::Error::None);

    std::cout << "ActivityPub OK" << std::endl;
  }

  //{
  //  const string src = R"({
  //    "a": [,,,])";

  //  auto res = json::Parser(src).parse();
  //  assert(res.error == json::Error::SyntaxError);

  //  std::cout << "Bad OK" << std::endl;
  //}

  return 0;
}
