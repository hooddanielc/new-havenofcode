import Ember from 'ember';

export default Ember.Component.extend({
  file: null,
  savingInitialPromise: false,
  queue: Ember.A(),
  store: Ember.inject.service('store'),

  selectedFiles: function () {
    const files = this.$('input[type="file"]')[0].files;
    const result = [];

    for (let i = 0; i < files.length; ++i) {
      result.push(this.get('store').createRecord('file', {
        name: files[i].name,
        bytes: files[i].size,
        type: files[i].type
      }));
    }

    return result;
  }.property('file'),

  actions: {
    upload: function () {
      const files = this.$('input[type="file"]')[0].files;
      window.files = files;

      if (files[0].size < 5000000) {
        throw new Error('choose a file that is at least five megabytes');
      }

      var fd = new FormData();
      fd.append('fname', 'arch.iso');
      fd.append('data1', files[0].slice(0, 5000000));
      fd.append('data2', files[0].slice(0, 5000000));
      console.log('uploading 5 megabyte file');
      Ember.$.ajax({
        type: 'POST',
        url: '/api/echo',
        data: fd,
        processData: false,
        contentType: false
      }).done(function(data) {
        console.log(data);
      });

      console.log(files);
    }
  }
});
