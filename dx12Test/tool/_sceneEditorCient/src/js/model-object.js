const CilentServer = require('./client-server.js');
const ModelBase = require('./model-common.js');

class Objects extends ModelBase.Class {
  constructor() {
    super();
  }
  create() {
    return {
      name: '',
      orientation: {
        pos: [0,0,0],
        rot: [0,0,0,0],
        scl: [0,0,0]
      },
      model:'',
      render_phase: []
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
