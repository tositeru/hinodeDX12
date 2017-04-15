const ClientServer = require('./js/client-server.js');

const riot = require('riot');
const ConnectUI = require('./tag/ui-connect.js');
const ObjectUI = require('./tag/ui-object.js');

const Object = require('./js/model-object.js');

const connectInfo = riot.mount('connect-ui')[0];

var initObj = Object.create();
initObj.name = 'test';
initObj.model = 'Apple';
initObj.render_phase.push({
  phaseName: "forward",
  pipelineName: "PTNC",
});
const objectUI = riot.mount('object-ui', initObj)[0];

connectInfo.appendMessage('メッセージの追加');

//処理開始
ClientServer._messageHandle = function(websocket, event) {
  connectInfo.appendMessage(event.data);
  return ;
  try {
    json = JSON.parse(event.data);
    if(json["objects"]) {
      //オブジェクト
      for( let obj of json["objects"] ) {
        let add = Objects.create(obj["name"], obj.orientation.pos, obj.orientation.rot, obj.orientation.scl, obj.model, obj.render_phase);
        Objects.add(add);
      }
      ObjectUI.renderDatalist();
    }
  } catch(e) {
    //JSONデータでなければ、そのまま出す
    var tag = document.getElementById("recieve_contents");
    tag.textContent = event.data;
    connectUI.appendMsg(event.data);
  }
}
