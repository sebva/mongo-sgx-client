FROM sconecuratedimages/apps:mongodb-3.4.4-alpine

RUN wget -O tlsping.sh https://gist.github.com/sebva/2356e4577da511289471ba0c0e3364f8/raw/720606e4a1ababe8a19ea6f0e61a57554ee3f8f3/tlsping.sh && chmod +x tlsping.sh
