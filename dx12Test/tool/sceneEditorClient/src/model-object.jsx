const CilentServer = require('./client-server.jsx');
const ModelBase = require('./model-common.jsx');

class Objects extends ModelBase.Class {
  constructor() {
    super();
  }
  create(name, pos, rot, scale, modelKey, renderPhases) {
    return {
      name: name,
      orientation: {
        pos: pos,
        rot: rot,
        scl: scale
      },
      model: modelKey,
      render_phase: Object.assign({},renderPhases)
    }
  }
  addAndSend(obj) {
    super.add(obj);
    let json = {
      add_objects: [obj],
    }
    let jsonText = JSON.stringify(json);
    CilentServer.send(jsonText);
  }
  deleteAndSend(name) {
    super.delete(name);
    let json = {
      del_objects: [name],
    }
    CilentServer.send(JSON.stringify(json));
  }
  updateAndSend(name, updateData) {
    super.update(name, updateData);

    updateData['name'] = name;
    let json = {
      update_objects: [ updateData ]
    }
    CilentServer.send(JSON.stringify(json));
  }
}

module.exports = new Objects();
