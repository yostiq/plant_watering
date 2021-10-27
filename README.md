# To setup

git clone https://github.com/yostiq/plant_watering
cd plant_watering
python3 -m venv venv
pip install -r requirements.txt
gunicorn -w4 -b <LOCAL IP>:5000 app:app
