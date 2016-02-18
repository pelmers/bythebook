serve:
	mkdir -p sessions
	virtualenv venv
	source venv/bin/activate; \
	pip install -r requirements.txt; \
	SPINE_CROP_BIN=`pwd`/../build/spinecrop python app.py
