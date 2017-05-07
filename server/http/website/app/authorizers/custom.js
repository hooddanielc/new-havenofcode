// app/authorizers/custom.js
import Base from 'ember-simple-auth/authorizers/base';

export default Base.extend({
  authorize(sessionData, block) {
    console.log('sessionData', sessionData);
    console.log('sessionBlob', block);
  }
});