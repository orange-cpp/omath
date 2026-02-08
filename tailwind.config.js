module.exports = {
  content: [
    './docs/**/*.md',
    './docs/**/*.html',
    './site/**/*.html',
    './docs/styles/tailwind.css'
  ],
  theme: {
    extend: {
      backdropBlur: {
        'lg': '16px',
      },
      colors: {
        glass: 'rgba(255,255,255,0.18)',
      },
    },
  },
  plugins: [],
};
