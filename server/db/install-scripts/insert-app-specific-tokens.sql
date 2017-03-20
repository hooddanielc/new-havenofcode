BEGIN;
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";
INSERT INTO "app_token" (id, refresh_token) VALUES ('no_reply_email', 'xxxxxx');
END;
