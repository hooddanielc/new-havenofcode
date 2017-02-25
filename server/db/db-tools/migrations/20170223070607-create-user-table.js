'use strict';

module.exports = {
  up: function (queryInterface, Sequelize) {
    return queryInterface.createTable('user', {
      id: {
        type: Sequelize.INTEGER,
        primaryKey: true,
        autoIncrement: true
      },
      createdAt: {
        type: Sequelize.DATE,
        defaultValue: Sequelize.literal('NOW()')
      },
      updatedAt: {
        type: Sequelize.DATE,
        defaultValue: Sequelize.literal('NOW()')
      },
      email: {
        type: Sequelize.STRING,
        unique: true,
        allowNull: false
      },
      github_id: {
        type: Sequelize.INTEGER,
        unique: true
      }
    });
  },

  down: function (queryInterface, Sequelize) {
    return queryInterface.dropTable('user');
  }
};
