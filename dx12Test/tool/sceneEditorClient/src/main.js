//以下のライブラリが必要になります
//npm install react react-dom react-tools browserify reactify gulp vinyl-source-stream

//インポート
var ClientServer = require('./client-server.jsx');
var glMatrix = require('./gl-matrix/dist/gl-matrix-min.js');

var Objects = require('./model-object.jsx');
const Models = require('./model-model.jsx');

//UI関係のインポート
var React = require('react');
var ReactDOM = require('react-dom');
var ConnectUI = require('./ui-connect.jsx');
var ObjectUI = require('./ui-object.jsx');
const ModelUI = require('./ui-model.jsx');

ReactDOM.render(
  React.createElement(ConnectUI.component),
  document.getElementById('connect-ui')
);

//モデルのテスト
// let materialParam = Models.createMaterial([1, 1, 1], 1, [1, 1, 1], 10, {dif:'data/panda.dds'});
// let initModel = Models.createSystem('test', 'tetra', 'ptnc', materialParam);
// Models.add(initModel);
// ModelUI.renderDatalist();
// ModelUI.render(initModel);

//オブジェクトのテスト
var renderPhase = {
  forward: 'PTNC',
};
var test = glMatrix.vec3.create();
let initObj = Objects.create('test', glMatrix.vec3.create(), [10, 11, 12, 13], [5, 6, 7], 'model', renderPhase);
Objects.add(initObj);
ObjectUI.renderDatalist();
ObjectUI.render(initObj);

//処理開始
ClientServer._messageHandle = function(websocket, event) {
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
  }
}
