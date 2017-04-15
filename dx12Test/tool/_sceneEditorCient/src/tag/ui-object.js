
(function(tagger) {
  if (typeof define === 'function' && define.amd) {
    define(function(require, exports, module) { tagger(require('riot'), require, exports, module)})
  } else if (typeof module !== 'undefined' && typeof module.exports !== 'undefined') {
    tagger(require('riot'), require, exports, module)
  } else {
    tagger(window.riot)
  }
})(function(riot, require, exports, module) {
const ClientServer = require('../js/client-server.js');
const vectorTag = require('./component-vector.js');
const Objects = require('../js/model-object.js');

riot.tag2('object-ui', '<p>オブジェクト</p> <div> <label>name:</label> <input riot-value="{this.obj.name}" oninput="{changeName}" type="text"> <input if="{!this.isExist}" onclick="{addObject}" type="button" value="この名前のオブジェクトを追加"> <input if="{this.isExist}" onclick="{deleteObject}" type="button" value="この名前のオブジェクトを削除"> </div> <vector name="pos" title="pos:" values="{this.obj.orientation.pos}" mixin_name="{this.mixinName}"></vector> <vector name="rot" title="rot:" values="{this.obj.orientation.rot}" mixin_name="{this.mixinName}"></vector> <vector name="scl" title="scale:" values="{this.obj.orientation.scl}" mixin_name="{this.mixinName}"></vector> <div> <label>model:</label> <input riot-value="{this.obj.model}" oninput="{updateValue}" name="model" type="text"> </div> <div> <label>render-phase:</label> <div each="{this.obj.render_phase}"> <input riot-value="{phaseName}" oninput="{updateRenderPhaseElement}" name="render-phase-name" type="text"> = <input riot-value="{pipelineName}" oninput="{updateRenderPhaseElement}" name="pipeline-name" type="text"> <input onclick="{deleteRenderPhase}" type="button" value="削除"> </div> <input onclick="{addRenderPhase}" type="button" value="描画フェーズの追加"> </div>', '', '', function(opts) {
    this.ieExist = false
    this.obj = {
      name: opts.name,
      orientation: {
        pos: opts.orientation.pos,
        rot: opts.orientation.rot,
        scl: opts.orientation.scl
      },
      model: opts.model,
      render_phase: opts.render_phase,
    }

    var Mixin = {
      init: function() {
        this.obs = riot.observable();
        this.obs.on('update', this.updateObject);
      },
      fireUpdate: function(key, value) {
        this.obs.trigger('update', key, value)
      },

      updateObject: function(key, value){
        console.log("Mixin.updateObject");
        console.log(key);
        console.log(value);

        var updateData = {};
        switch(key) {
          case 'pos':
            this.obj.orientation.pos = value.slice();
            updateData.orientation = {}
            updateData.orientation.pos = value.slice();
            break;
          case 'rot':
            this.obj.orientation.rot = value.slice();
            updateData.orientation = {}
            updateData.orientation.rot = value.slice();
            break;
          case 'scl':
            this.obj.orientation.scl = value.slice();
            updateData.orientation = {}
            updateData.orientation.scl = value.slice();
            break;
          case 'model':
            this.obj.model = value;
            updateData.model = value;
            break;
        }

        if(this.isExist) {
          Objects.update(this.obj.name, updateData);
        }
        var obj = Objects.find('test');
        console.log(obj);
      },
    }
    this.mixin(Mixin);
    Mixin.updateObject = Mixin.updateObject.bind(this);
    this.mixinName = 'object-common';
    riot.mixin(this.mixinName, Mixin);

    this.changeName = function(event) {
      console.log('ui-object: changeName')
      let name = event.target.value;
      let obj = Objects.find(name);
      this.isExist = (undefined != obj);
      if(this.isExist) {

        Objects.deepUpdate(this.obj, obj);
        this.update(this.obj);
      }else{
        this.obj.name = name
      }
    }.bind(this)
    this.updateValue = function(event) {
      console.log(event);
      this.fireUpdate(event.target.name, event.target.value);
    }.bind(this)
    this.addObject = function(event) {
      console.log('ui-object: addObject')
      let newObject = Objects.create();
      Objects.deepUpdate(newObject, this.obj);
      Objects.add(newObject)
      this.isExist = true;
    }.bind(this)
    this.deleteObject = function(event) {
      console.log('ui-object: deleteObject')
      Objects.delete(event.obj.name);
      this.isExist = false;
    }.bind(this)
    this.updateRenderPhaseElement = function(event) {
      console.log(event)
      for(var i=0; i<this.obj.render_phase.length; ++i) {
        let v = this.obj.render_phase[i];
        if( v.phaseName == event.item.phaseName ) {
          switch(event.target.name) {
            case 'render-phase-name': v.phaseName = event.target.value; break;
            case 'pipeline-name':     v.pipelineName = event.target.value; break;
          }
        }
      }

      if(this.isExist) {
        Objects.update(this.obj.name, {render_phase: this.obj.render_phase});
      }
    }.bind(this)
    this.addRenderPhase = function(event) {
      console.log('ui-object: addRenderPhase')
      this.obj.render_phase.push({phaseName: 'unknown', pipelineName:'unknown'});

      if(this.isExist) {
        Objects.update(this.obj.name, {render_phase: this.obj.render_phase});
      }
    }.bind(this)
    this.deleteRenderPhase = function(event) {
      console.log('ui-object: deleteRenderPhase')
      let targetPhase = event.item;
      for(var i=0; i<this.obj.render_phase.length; ++i) {
        let v = this.obj.render_phase[i];
        if(v.phaseName == targetPhase.phaseName) {
          this.obj.render_phase.splice(i, 1);
          break;
        }
      }

      if(this.isExist) {
        Objects.update(this.obj.name, {render_phase: this.obj.render_phase});
      }
    }.bind(this)

});
});