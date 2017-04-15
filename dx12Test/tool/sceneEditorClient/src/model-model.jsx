const CilentServer = require('./client-server.jsx');
const ModelBase = require('./model-common.jsx');

class Models extends ModelBase.Class
{
  constructor() {
    super();
  }
  createSystem(name, primitive, vertex, material) {
    return {
      name: name,
      system: {
        prm: primitive,
        vertex: vertex,
        mtl: Object.assign({}, material)
      },
    }
  }
  createMaterial(diffiuse, alpha, specular, specularPower, textures) {
    let result = {
      dif: diffiuse,
      alp: alpha,
      spc: specular,
      pow: specularPower
    };
    //テクスチャの設定
    if( textures.dif ) {
      result.dif_tex = textures.dif;
    }
    return result;
  }
  addAndSend(obj) {
    this.add(obj);
    let json = {
      add_models: [obj],
    }
    let jsonText = JSON.stringify(json);
    // CilentServer.send(jsonText);
  }
  deleteAndSend(name) {
    this.delete(name);
    let json = {
      del_models: [name],
    }
    // CilentServer.send(JSON.stringify(json));
  }
  updateAndSend(name, updateData) {
    this.update(name, updateData);

    updateData['name'] = name;
    let json = {
      update_models: [ updateData ]
    }
    // CilentServer.send(JSON.stringify(json));
  }
}

module.exports = new Models();
