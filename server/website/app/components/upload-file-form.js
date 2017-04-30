import Ember from 'ember';

export default Ember.Component.extend({
  file: null,
  savingInitialPromise: false,
  queue: Ember.A(),
  store: Ember.inject.service('store'),

  createPromises: function (files) {
    const result = [];

    for (let i = 0; i < files.length; ++i) {
      result.push(this.get('store').createRecord('file', {
        name: files[i].name,
        bytes: files[i].size,
        type: files[i].type
      }));
    }

    return Ember.RSVP.Promise.all(result.map((file, i) => {
      return file.save().then((res) => {
        res.fileObject = files[i];
        return res;
      });
    }));
  },

  uploadFile: function (file, fileObject) {
    const parts = file.get('fileParts');
    let start = 0;
    let end = 0;

    const pop = () => {
      const part = parts.shiftObject();

      if (part) {
        end += part.get('bytes');

        const formData = new FormData();
        formData.append('blob', fileObject.slice(start, end));
        formData.append('blob2', fileObject.slice(start, end));
        formData.append('regularData', 'just a test');
        formData.append('somejson', JSON.stringify({
          hey: 'girl',
          how: 'it goin'
        }));

        return new Ember.RSVP.Promise((resolve, reject) => {
          Ember.$.ajax({
            type: 'put',
            url: `/api/file-part/${part.get('id')}`,
            data: formData,
            processData: false,
            contentType: false
          }).done(function (data) {
            console.log(data);
            resolve(data);
          }).fail(function (err) {
            console.log(err);
            reject(err);
          });
        }).then(() => {
          start = end;
          return pop();
        });
      } else {
        console.log('saving file');
        file.set('status', 'complete');
        return file.save();
      }
    };

    return pop();
  },

  actions: {
    upload: function () {
      const files = this.$('input[type="file"]')[0].files;

      this.createPromises(files).then((res) => {
        const pop = () => {
          const record = res.pop();

          if (record) {
            return this.uploadFile(record, record.fileObject).then(() => {
              return pop();
            });
          }
        };

        return pop();
      });
    }
  }
});
