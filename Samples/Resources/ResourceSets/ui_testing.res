set_name = "ui_testing"
shader_program = {
	resource_name = "TextShader"
	shader = {
		type = "vertex"
		path = "..\..\..\Samples\Resources\Shaders\TextShader.vertexshader"
	}
	shader = {
		type = "fragment"
		path = "..\..\..\Samples\Resources\Shaders\TextShader.fragmentshader"
	}
}

shader_program = {
	resource_name = "TextureShader"
	shader = {
		type = "vertex"
		path = "..\..\..\Samples\Resources\Shaders\TextureShader.vertexshader"
	}
	shader = {
		type = "fragment"
		path = "..\..\..\Samples\Resources\Shaders\TextureShader.fragmentshader"
	}
}

font = {
	resource_name = "Arial_en"
	path = "..\..\..\Samples\Resources\Fonts\arial.ttf"
	codes = L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
	pixel_width = 0
	pixel_height = 48
}

font = {
	resource_name = "Arial_ru"
	path = "..\..\..\Samples\Resources\Fonts\arial.ttf"
	codes = L" !\"#$%&'()*+,-./0123456789:;<=>?@АБВГДАЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЬЭЮЯ[\\]^_`абвгдежзийклмнопрстуфхцчшщъыьэюя{|}~"
	pixel_width = 0
	pixel_height = 48
}

font = {
	resource_name = "Arial"
	path = "..\..\..\Samples\Resources\Fonts\arial.ttf"
	codes = L" !\"#$%&'()*+,-./0123456789:;<=>?@АБВГДАЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЬЭЮЯ[\\]^_`абвгдежзийклмнопрстуфхцчшщъыьэюя{|}~ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
	pixel_width = 0
	pixel_height = 48
}

texture = {
	resource_name = "SimpleButton"
	path = "..\..\..\Samples\Resources\Textures\SimpleButton.png"
}

texture = {
	resource_name = "SimpleButton_pressed"
	path = "..\..\..\Samples\Resources\Textures\SimpleButton_pressed.png"
}