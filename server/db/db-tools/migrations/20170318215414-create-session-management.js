'use strict';

module.exports = {
  up: function (queryInterface, Sequelize) {
    return queryInterface.createTable('session', {
      id: {
        type: Sequelize.UUID,
        primaryKey: true,
        defaultValue: Sequelize.UUIDV1
      },
      createdAt: {
        type: Sequelize.DATE,
        defaultValue: Sequelize.literal('NOW()')
      },
      updatedAt: {
        type: Sequelize.DATE,
        defaultValue: Sequelize.literal('NOW()')
      },
      deleted: {
        type: Sequelize.BOOLEAN,
        defaultValue: false
      },
      user: {
        type: Sequelize.INTEGER,
        references: { model: 'user', key: 'id' },
        allowNull: false
      }
    }).then(() => {
      return queryInterface.createTable('session_ip_log', {
        id: {
          type: Sequelize.UUID,
          primaryKey: true,
          defaultValue: Sequelize.UUIDV1
        },
        createdAt: {
          type: Sequelize.DATE,
          defaultValue: Sequelize.literal('NOW()')
        },
        updatedAt: {
          type: Sequelize.DATE,
          defaultValue: Sequelize.literal('NOW()')
        },
        deleted: {
          type: Sequelize.BOOLEAN,
          defaultValue: false
        },
        session: {
          type: Sequelize.UUID,
          references: { model: 'session', key: 'id' },
          allowNull: false
        },
        ip: {
          type: Sequelize.STRING,
          allowNull: false
        },
        userAgent: {
          type: Sequelize.STRING,
          allowNull: false
        }
      });
    });
  },

  down: function (queryInterface, Sequelize) {
    return queryInterface.dropTable('session_id_log').then(() => {
      return queryInterface.dropTable('session');
    });
  }
};
