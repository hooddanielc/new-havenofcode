import Ember from 'ember';

export default Ember.Component.extend({
  store: Ember.inject.service('store'),
  filesInProgress: Ember.A(),

  createPromises: function (files) {
    const result = Ember.A();

    for (let i = 0; i < files.length; ++i) {
      result.pushObject(this.get('store').createRecord('file', {
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
    this.get('files').pushObject(file._internalModel);
    window.theThis = this;
    const parts = file.get('fileParts');
    let start = 0;
    let end = 0;

    const pop = () => {
      const part = parts.shiftObject();

      if (part) {
        end += part.get('bytes');

        const formData = new FormData();
        const blob = fileObject.slice(start, end);
        formData.append('blob', blob);

        return new Ember.RSVP.Promise((resolve, reject) => {
          Ember.$.ajax({
            type: 'put',
            url: `/api/file-parts/${part.get('id')}`,
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
