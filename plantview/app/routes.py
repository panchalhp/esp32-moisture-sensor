
from app import app
from flask import request, Response


@app.route('/update-sensor', methods=['POST'])
def update():
	data = request.get_json()
	ts = data['timestamp']
	values = data['values']

	print('Timestamp: {}'.format(int(ts)))
	for value in values:
		print(
			'PlantName: {} SensorPin: {} MoistureValue: {}'.format(
				value['plantName'],
				value['sensorPinNumber'],
				value['moistureValue']
			)
		)
	print('-'*50)

	return Response(status=200)




