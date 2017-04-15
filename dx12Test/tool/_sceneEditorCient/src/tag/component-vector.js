
(function(tagger) {
  if (typeof define === 'function' && define.amd) {
    define(function(require, exports, module) { tagger(require('riot'), require, exports, module)})
  } else if (typeof module !== 'undefined' && typeof module.exports !== 'undefined') {
    tagger(require('riot'), require, exports, module)
  } else {
    tagger(window.riot)
  }
})(function(riot, require, exports, module) {
riot.tag2('vector', '<div> <label>{opts.title}</label> <input each="{value, index in this.elements}" name="{index}" riot-value="{value}" oninput="{changeValue}" type="{\'number\'}"> </div>', '', '', function(opts) {
    this.name = opts.name;
    this.elements = opts.values;

    if( opts.mixin_name ) {
      console.log(`component-vector: set mixin! name=${opts.mixin_name}`);
      this.mixin(opts.mixin_name);
    }

    this.changeValue = function(event){
      console.log(`Vector changeValue name=${this.name}`)

      var v = parseFloat(event.target.value);
      if(isFinite(v)) {
        this.elements[event.item.index] = v;
      }
      this.fireUpdate(this.name, this.elements);
    }.bind(this)
});
});